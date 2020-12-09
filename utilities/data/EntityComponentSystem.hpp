#pragma once

#include <v4d.h>

/**
 * V4D's Entity-Component system v1.0
 * Author: Olivier St-Laurent, December 2020
 * 
 * This is a very lightweight and elegant implementation of a data-oriented entity-component workload for very fast computation.
 * It is thread-safe and optimized for looping through all components of certain types instead of entities.
 * You can fine your own entities and components in a very elegant way without bloating your code.
 * It is important to note that we cannot do inheritance because this will defeat the purpose of Entity-Component Systems.
 * 
 * This is especially good for defining GameObjects that have components that get traversed every frame.
 * 
 * You may define any number of entity classes, each with completely custom components types.
 * This uses lots of MACROS and Templates to "automatically generate code" from very simple and elegant definitions.
 * 
 * Usage:

 // MyEntity.h
 
	struct SomeDataStruct {
		int a;
		int b;
	};

	class MyEntity {
		V4D_ENTITY_DECLARE_CLASS(MyEntity)
		V4D_ENTITY_DECLARE_COMPONENT(MyEntity, std::string, firstName)
		V4D_ENTITY_DECLARE_COMPONENT(MyEntity, glm::mat4, transform)
		V4D_ENTITY_DECLARE_COMPONENT(MyEntity, SomeDataStruct, someData)
		//... more components
	};

 // MyEntity.cpp

	V4D_ENTITY_DEFINE_CLASS(MyEntity)
	V4D_ENTITY_DEFINE_COMPONENT(MyEntity, std::string, firstName)
	V4D_ENTITY_DEFINE_COMPONENT(MyEntity, glm::mat4, transform)
	V4D_ENTITY_DEFINE_COMPONENT(MyEntity, SomeDataStruct, someData)
	//... more components

 // main.cpp
	
	// this creates and returns a shared pointer to a new entity
	auto entity = MyEntity::Create();
	// You cannot create any constructor for the entity as it is made opaque by this system.
	// However, if you want to construct things upon instantiation with custom arguments, you may have an operator(...) overload and arguments you pass to Create(...) will be forwarded to it. Passing no argument to Create() will NOT call operator().
	
	// The entity does not get destroyed when we leave the scope, it will live forever until we manually destroy them in one of three ways:
		
		// 1. Loop through each entity to determine which one to destroy and simply call ->Destroy() on the entity
		
			MyEntity::ForEach([](auto entity){
				entity->Destroy();
			});
			
		// 2. the ClearAll static member
		
			MyEntity::ClearAll();
			
		// 3. Call Destroy with a given entity instance index
		
			MyEntity::Destroy(index);
			
		// All three ways are equivalent
		
	// Note: since they are shared pointers, if a reference exists somewhere else it will not get destroyed until all references are destroyed, but its index WILL be invalidated.
	
	// We can get the index of an entity like this:
	int32_t index = entity->GetIndex();
	// This index can then be used to fetch that entity again like this:
	auto entity = MyEntity::Get(index);
	// Entity indices will not change for a given entity as long as you don't destroy it. Then their indices will be reused by new entities, hence invalidated.
	
	// Entities may have any number of Components. 
	// Each component is referenced through an opaque pointer in the entity but exposes a few practical features for indirect access.
	// Component member references in entities are simply defined as the name of the component.
	// Entities also contain member methods like Add_*() and Remove_*()
	
	// add a 'firstName' component which is defined above as a std::string, and pass "Bob" in its constructor
	entity->Add_firstName("Bob");
	// The component is constructed in-place with the given arguments. 
	// If the component had already been added to that entity, this will do nothing and ignore the given arguments.
	// You may also pass initializer lists if the component supports it
	
	// You may assign the value of the component using its reference directly like this and it will effectively use the = operator on the component type:
	entity->firstName = "Bob";
	// however, if the component has not been added first, this will do nothing. Same goes for removing the component, setting it to "" will NOT remove it.
	
	// We may get a locked reference on a component like this:
	auto someFirstNameRef = entity->someData.Lock();
	// This effectively locks the entire list of components until it is destroyed by going out of scope. Must not try to lock two components of the same type without destroying the previous one.
	// Must also check if it is valid by using its boolean explicit cast operator, then you may access its values using the arrow operator like this: 
	if (someFirstNameRef) someFirstNameRef->a = 5;
	// May force the unlock by calling someFirstNameRef.Unlock();
	
	// we may remove the component like this. This actually removes the component, it doesn't just assign it to "".
	entity->Remove_firstName();
	// If the component was already not present, this does nothing
	
	// Entities also have static members *Components for every component defined in it, to access component lists.
	
	// We can loop through all firstNames like this:
	MyEntity::firstNameComponents.ForEach([](int32_t entityInstanceIndex, auto& firstName){
		std::cout << firstName << std::endl;
	});
	// This will only run for existing components, not for entities that don't have that component added to it
	// All components of the same type are contiguous in memory for great CPU cache usage
	// Hence it is preferable to not use pointers as component types because it will defeat the purpose
	
	// We can create lots of entities and assign them components like this:
	MyEntity::Create()->Add_firstName("Jane")->Add_transform({1});
	MyEntity::Create()
		->Add_firstName("Joe")
		->Add_transform({1})
		->Add_someData(34,66);
	
	// All Add_* members will always return a pointer to the entity, not the component itself.
	// This is obviously practical for writing chained code like above, but is also because we cannot guarantee that a reference to a component will not become invalidated.
	// It is also important to note that only the initial Create() method will return a shared_ptr, any following Add_* method will return the raw pointer.
	// Components will always be moving around in memory, hence it is not safe to get references or pointers to them, instead we use the safer functions 'Do' and 'ForEach' to modify them.
	// Component indices are thus completely opaque and should not be used.
	// Also, it is preferable to use trivially constructible/destructible components that can be moved around easily without having their constructor/destructor doing heavy lifting stuff many times
	
	// Direct component member access will cast to boolean so that we can check if an entity has a specific component:
	if (entity->someData) {
		// this entity has a 'someData' component
	}
	
	// We can quickly access a specific value from a component using the arrow operator,
	// but keep in mind that this is not thread safe so try to keep it atomic, and also this will segfault if that entity does not have that component.
	entity->someData->a = 55;
	
	// To modify the values of a component from a specific entity in a thread-safe manner, we can do it like this: 
	entity->firstName->Do([](auto& firstName){
		firstName = "Paul";
	});
	entity->someData->Do([](auto& someData){
		someData.a = 44;
	});
	// This also ensures that the component exists before running the code, so you may run this with any entity without worrying if the component is present.
	// The 'Do' method also returns true if the entity has that component, false otherwise. 
	
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Components wrapper template

namespace v4d::data::EntityComponentSystem {
	template<typename EntityClass, typename ComponentType>
	class Component {
	friend EntityClass;
		mutable std::recursive_mutex componentsMutex;
		struct ComponentTuple {
			int32_t entityInstanceIndex;
			ComponentType component;
			template<typename...Args>
			ComponentTuple(int32_t i, Args&&...args) : entityInstanceIndex(i), component(std::forward<Args>(args)...) {}
			template<typename T, int N>
			ComponentTuple(int32_t i, const T (&arr)[N]) : entityInstanceIndex(i), component(arr) {}
		};
		std::vector<ComponentTuple> componentsList;
		template<typename...Args>
		int32_t Add(int32_t entityInstanceIndex, Args&&...args) {
			std::lock_guard lock(componentsMutex);
			componentsList.emplace_back(entityInstanceIndex, std::forward<Args>(args)...);
			return componentsList.size() - 1;
		}
		template<typename List_T>
		int32_t Add(int32_t entityInstanceIndex, std::initializer_list<List_T>&& list) {
			std::lock_guard lock(componentsMutex);
			componentsList.emplace_back(entityInstanceIndex, std::forward<std::initializer_list<List_T>>(list));
			return componentsList.size() - 1;
		}
		template<typename ArrayRef, int ArrayN>
		int32_t Add(int32_t entityInstanceIndex, const ArrayRef (&val)[ArrayN]) {
			std::lock_guard lock(componentsMutex);
			componentsList.emplace_back(entityInstanceIndex, val);
			return componentsList.size() - 1;
		}
		int32_t Remove(int32_t componentIndex) {
			if (componentIndex != -1) {
				std::lock_guard lock(componentsMutex);
				if ((size_t)componentIndex < componentsList.size()-1) {
					// Move the last element to the position we want to delete and return its instance index so that we can adjust it in the calling method
					componentsList[componentIndex] = std::move(componentsList.back());
					componentsList.pop_back();
					return componentsList[componentIndex].entityInstanceIndex;
				}
				if (componentsList.size() > 0) componentsList.pop_back();
			}
			return -1;
		}
		ComponentType* Get(int32_t componentIndex) {
			if (componentIndex == -1) return nullptr;
			std::lock_guard lock(componentsMutex);
			if ((size_t)componentIndex < componentsList.size()) {
				return &componentsList[componentIndex].component;
			}
			return nullptr;
		}
	public:
		void ForEach(std::function<void(int32_t& entityInstanceIndex, ComponentType& data)>&& func) {
			std::lock_guard lock(componentsMutex);
			for (auto&[entityInstanceIndex, data] : componentsList) {
				func(entityInstanceIndex, data);
			}
		}
		void ForEach_LockEntities(std::function<void(int32_t& entityInstanceIndex, ComponentType& data)>&& func) {
			auto entitiesLock = EntityClass::GetLock();
			std::lock_guard lock(componentsMutex);
			for (auto&[entityInstanceIndex, data] : componentsList) {
				func(entityInstanceIndex, data);
			}
		}
		size_t Count() const {
			std::lock_guard lock(componentsMutex);
			return componentsList.size();
		}
		bool Do(int32_t componentIndex, std::function<void(ComponentType& data)>&& func){
			if (componentIndex == -1) return false;
			std::lock_guard lock(componentsMutex);
			if ((size_t)componentIndex < componentsList.size()) {
				func(componentsList[componentIndex].component);
				return true;
			}
			return false;
		}
		
		template<typename T>
		void Set(int32_t componentIndex, const T& val){
			if (componentIndex == -1) return;
			std::lock_guard lock(componentsMutex);
			if (componentIndex < componentsList.size()) {
				componentsList[componentIndex].component = val;
			}
		}
		template<typename T>
		void Set(int32_t componentIndex, T&& val){
			if (componentIndex == -1) return;
			std::lock_guard lock(componentsMutex);
			if (componentIndex < componentsList.size()) {
				componentsList[componentIndex].component = std::move(val);
			}
		}
		class ComponentReferenceLocked {
			friend Component;
			std::unique_lock<std::recursive_mutex> lock;
			ComponentType* ptr;
			ComponentReferenceLocked() : lock(), ptr(nullptr) {}
			ComponentReferenceLocked(std::unique_lock<std::recursive_mutex>& lock, ComponentType* ptr) : lock(std::move(lock)), ptr(ptr) {}
			DELETE_COPY_MOVE_CONSTRUCTORS(ComponentReferenceLocked)
			public:
			operator bool() {return !!ptr;}
			ComponentType* operator->() {return ptr;}
			void Unlock() {
				ptr = nullptr;
				lock = {};
			}
		};
		ComponentReferenceLocked Lock(int32_t componentIndex) {
			std::unique_lock lock(componentsMutex);
			if (componentIndex == -1 || (size_t)componentIndex >= componentsList.size()) return ComponentReferenceLocked{};
			return ComponentReferenceLocked{lock, &componentsList[componentIndex].component};
		}
	};
}

/////////////////////////////////////////////////////////
// MACROS for custom entity class definitions

// Used in .h files
#define V4D_ENTITY_DECLARE_CLASS(ClassName)\
	private:\
		static std::recursive_mutex entityInstancesMutex;\
		static std::vector<std::shared_ptr<ClassName>> entityInstances;\
		int32_t index;\
		ClassName(int32_t index);\
	public:\
		static std::shared_ptr<ClassName> Create();\
		template<typename...Args>\
		static std::shared_ptr<ClassName> Create(Args&&...args) {\
			std::lock_guard lock(entityInstancesMutex);\
			size_t nbEntityInstances = entityInstances.size();\
			for (size_t i = 0; i < nbEntityInstances; ++i) {\
				if (!entityInstances[i]) {\
					auto* e = new ClassName(i);\
					(*e)(std::forward<Args>(args)...);\
					return entityInstances[i] = std::shared_ptr<ClassName>(e);\
				}\
			}\
			auto* e = new ClassName(nbEntityInstances);\
			(*e)(std::forward<Args>(args)...);\
			return entityInstances.emplace_back(e);\
		}\
		static void Destroy(int32_t index);\
		void Destroy();\
		static void ClearAll();\
		static size_t Count();\
		static std::unique_lock<std::recursive_mutex> GetLock();\
		static void ForEach(std::function<void(std::shared_ptr<ClassName>)>&& func);\
		static std::shared_ptr<ClassName> Get(int32_t entityInstanceIndex);\
		inline int32_t GetIndex() const {return index;};

// Used in .cpp files
#define V4D_ENTITY_DEFINE_CLASS(ClassName)\
	std::recursive_mutex ClassName::entityInstancesMutex {};\
	std::vector<std::shared_ptr<ClassName>> ClassName::entityInstances {};\
	ClassName::ClassName(int32_t index) : index(index) {}\
	std::shared_ptr<ClassName> ClassName::Create() {\
		std::lock_guard lock(entityInstancesMutex);\
		size_t nbEntityInstances = entityInstances.size();\
		for (size_t i = 0; i < nbEntityInstances; ++i) {\
			if (!entityInstances[i]) return entityInstances[i] = std::shared_ptr<ClassName>(new ClassName(i));\
		}\
		return entityInstances.emplace_back(new ClassName(nbEntityInstances));\
	}\
	void ClassName::Destroy(int32_t index) {\
		std::lock_guard lock(entityInstancesMutex);\
		if (index > -1 && index < entityInstances.size()) {\
			entityInstances[index]->index = -1;\
			entityInstances[index].reset();\
			if (index == entityInstances.size()-1) {\
				entityInstances.pop_back();\
				while (!entityInstances[entityInstances.size()-1]) {\
					entityInstances.pop_back();\
				}\
			}\
		}\
	}\
	void ClassName::Destroy() {\
		std::lock_guard lock(entityInstancesMutex);\
		Destroy(index);\
	}\
	void ClassName::ForEach(std::function<void(std::shared_ptr<ClassName>)>&& func) {\
		std::lock_guard lock(entityInstancesMutex);\
		for (auto& entity : entityInstances) {\
			if (entity) {\
				func(entity);\
			}\
		}\
	}\
	std::shared_ptr<ClassName> ClassName::Get(int32_t entityInstanceIndex) {\
		std::lock_guard lock(entityInstancesMutex);\
		if (entityInstanceIndex == -1 || (size_t)entityInstanceIndex >= entityInstances.size()) return nullptr;\
		return entityInstances[entityInstanceIndex];\
	}\
	void ClassName::ClearAll() {\
		std::lock_guard lock(entityInstancesMutex);\
		entityInstances.clear();\
	}\
	size_t ClassName::Count() {\
		std::lock_guard lock(entityInstancesMutex);\
		return entityInstances.size();\
	}\
	std::unique_lock<std::recursive_mutex> ClassName::GetLock() {\
		return std::unique_lock<std::recursive_mutex>{entityInstancesMutex};\
	}


/////////////////////////////////////////////////////////
// MACROS for component definitions within entity classes

// Used in .h files
#define V4D_ENTITY_DECLARE_COMPONENT(ClassName, ComponentType, MemberName) \
	class ComponentReference_ ## MemberName {\
		friend ClassName;\
		int32_t index;\
		ComponentReference_ ## MemberName () : index(-1) {}\
		ComponentReference_ ## MemberName (int32_t componentIndex) : index(componentIndex) {}\
		~ComponentReference_ ## MemberName () {\
			if (index != -1) {\
				std::lock_guard lock(ClassName::entityInstancesMutex);\
				int32_t replacementInstanceIndex = MemberName ## Components .Remove(index);\
				if (replacementInstanceIndex != -1 && ClassName::entityInstances[replacementInstanceIndex]) {\
					ClassName::entityInstances[replacementInstanceIndex]-> MemberName.index = index ;\
				}\
			}\
		}\
		ComponentType* Get() {\
			if (index == -1) return nullptr;\
			return MemberName ## Components .Get(index);\
		}\
		public:\
		operator bool() {return index != -1 && !!Get();}\
		template<typename T>\
		void operator = (T&& val) {\
			if (index != -1) MemberName ## Components .Set<ComponentType>(index, std::forward<T>(val));\
		}\
		ComponentType* operator -> () {return Get();}\
		bool Do(std::function<void(ComponentType&)>&& func){\
			if (index == -1) return false;\
			return MemberName ## Components .Do(index, std::forward<std::function<void(ComponentType&)>>(func));\
		}\
		v4d::data::EntityComponentSystem::Component<ClassName, ComponentType>::ComponentReferenceLocked Lock(){\
			return MemberName ## Components .Lock(index);\
		}\
	} MemberName;\
	static v4d::data::EntityComponentSystem::Component<ClassName, ComponentType> MemberName ## Components ;\
	template<typename...Args>\
	ClassName* Add_ ## MemberName (Args&&...args) {\
		std::lock_guard lock(ClassName::entityInstancesMutex);\
		if (MemberName.index == -1) MemberName.index = MemberName ## Components .Add<Args...>(index, std::forward<Args>(args)...);\
		return this;\
	}\
	template<typename List_T>\
	ClassName* Add_ ## MemberName (std::initializer_list<List_T>&& list) {\
		std::lock_guard lock(ClassName::entityInstancesMutex);\
		if (MemberName.index == -1) MemberName.index = MemberName ## Components .Add<List_T>(index, std::forward<std::initializer_list<List_T>>(list));\
		return this;\
	}\
	template<typename ArrayRef, int ArrayN>\
	ClassName* Add_ ## MemberName (const ArrayRef (&val)[ArrayN]) {\
		std::lock_guard lock(ClassName::entityInstancesMutex);\
		if (MemberName.index == -1) MemberName.index = MemberName ## Components .Add<ArrayRef, ArrayN>(index, val);\
		return this;\
	}\
	void Remove_ ## MemberName ();

// Used in .cpp files
#define V4D_ENTITY_DEFINE_COMPONENT(ClassName, ComponentType, MemberName) \
	v4d::data::EntityComponentSystem::Component<ClassName, ComponentType> ClassName::MemberName ## Components  {};\
	void ClassName::Remove_ ## MemberName () {\
		if (MemberName.index != -1) {\
			std::lock_guard lock(ClassName::entityInstancesMutex);\
			auto replacementInstanceIndex = MemberName ## Components .Remove(MemberName.index);\
			if (replacementInstanceIndex != -1 && ClassName::entityInstances[replacementInstanceIndex]) {\
				ClassName::entityInstances[replacementInstanceIndex]-> MemberName.index = MemberName.index ;\
			}\
			MemberName.index = -1;\
		}\
	}

