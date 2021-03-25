#pragma once

#include <v4d.h>

/**
 * V4D's Entity-Component system v1.1
 * Author: Olivier St-Laurent, December 2020 - March 2021
 * 
 * This is a very lightweight and elegant implementation of a data-oriented entity-component workload for very fast computation.
 * It is thread-safe and optimized for looping through all components of certain types instead of the full entities.
 * You can define your own entities and components in a very elegant way without bloating your code.
 * It is important to note that we cannot do component inheritance because this will defeat the purpose of this concept.
 * 
 * This is especially good for defining GameObjects that have components that get traversed every frame.
 * 
 * You may define any number of entity classes, each with completely custom components types.
 * This uses lots of MACROS and Templates to "automagically generate code" from very simple and elegant definitions.
 * 
 * Usage:

 // MyEntity.h
 
	struct SomeDataStruct {
		int a;
		int b;
	};

	class MyEntity {
		V4D_ENTITY_DECLARE_CLASS(MyEntity)
		V4D_ENTITY_DECLARE_COMPONENT(MyEntity, std::string, firstName) // OneToOne entity-component
		V4D_ENTITY_DECLARE_COMPONENT(MyEntity, glm::mat4, transform)
		V4D_ENTITY_DECLARE_COMPONENT(MyEntity, SomeDataStruct, someData)
		V4D_ENTITY_DECLARE_COMPONENT_MAP(MyEntity, std::string_view, SomeDataStruct, someDataMap) // OneToMany (with this feature, an entity can host a map of a certain type of components) documented at the end
		//... more components
	};

 // MyEntity.cpp

	V4D_ENTITY_DEFINE_CLASS(MyEntity)
	V4D_ENTITY_DEFINE_COMPONENT(MyEntity, std::string, firstName)
	V4D_ENTITY_DEFINE_COMPONENT(MyEntity, glm::mat4, transform)
	V4D_ENTITY_DEFINE_COMPONENT(MyEntity, SomeDataStruct, someData)
	V4D_ENTITY_DEFINE_COMPONENT_MAP(MyEntity, std::string_view, SomeDataStruct, someDataMap)
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
		
	// Note: since they are shared pointers, if a reference exists somewhere else it will not get destroyed until all references are destroyed, but its index WILL be invalidated and set to -1.
	
	// We can get the index of an entity like this:
	auto index = entity->GetIndex();
	// This index can then be used to fetch that entity again like this:
	auto entity = MyEntity::Get(index);
	// Entity indices will not change for a given entity as long as you don't destroy it. Then their indices will be reused by new entities, hence invalidated.
	// To verify if an entity has been destroyed, you may compare GetIndex() with -1.
	
	// Entities may have any number of Components. 
	// Each component is referenced through an opaque pointer in the entity but exposes a few practical features for indirect access.
	// Component member references in entities are simply defined as the name of the component.
	// Entities also contain member methods like Add_*() and Remove_*()
	
	// add a 'firstName' component which is defined above as a std::string, and pass "Bob" in its constructor
	entity->Add_firstName("Bob");
	// The component is constructed in-place with the given arguments. 
	// If the component had already been added to that entity, this will do nothing and ignore the given arguments.
	// You may also pass initializer lists if the component supports it in its constructor.
	// This function also returns a locked reference to the component.
	
	// We can also get a locked reference on a component like this:
	auto someFirstNameRef = entity->someData.Lock();
	// This effectively locks the entire list of components until it is unlocked by going out of scope. You must not try to lock two components of the same type without unlocking the previous one.
	// Must also check if it is valid by using its boolean explicit cast operator, then you may access its values using the arrow operator like this: 
	if (someFirstNameRef) someFirstNameRef->a = 5;
	// If the locked reference was retrieved using the Add_* function, you do not need to verify if it is valid (Added) since it is explicitly guaranteed to be already.
	// You may force the unlock by calling someFirstNameRef.Unlock() then the reference is invalidated.
	
	// we may remove the component like this. This actually removes the component, it doesn't just assign it to "".
	entity->Remove_firstName();
	// If the component was already not present, this does nothing
	
	// You may assign the value of the component using its reference directly like this and it will safely lock and use the = operator on the component while perfectly forwarding the value:
	entity->firstName = "Bob";
	// however, if the component has not been added first, this will do nothing. Same goes for removing the component, setting it to "" (or equivalent empty value for other types) will NOT remove the component.
	
	// Components will always be moving around in memory, hence it is not safe to get references or pointers to them, instead we use the safer functions 'Lock', 'Do' and 'ForEach' to lock them while we read and/or write to them.
	// Component indices are thus completely opaque and should not be used.
	// Also, it is preferable to use trivially constructible/destructible components that can be moved around easily without having their constructor/destructor doing heavy lifting stuff many times
	
	// Entities also have static members *Components for every component defined in it, to access component lists.
	
	// We can loop through all firstNames like this:
	MyEntity::firstNameComponents.ForEach([](auto entityInstanceIndex, auto& firstName){
		std::cout << firstName << std::endl;
	});
	// This will only run for existing components, not for entities that don't have that component added to it
	// All components of the same type are contiguous in memory for great CPU cache usage
	// Hence it is preferable to not use pointers as component types because it will defeat the purpose
	
	// Direct component member access will cast to boolean so that we can check if an entity has a specific component:
	if (entity->someData) {
		// this entity has a 'someData' component
		// However, it may already be invalidated if it has been removed in another thread
	}
	
	// To modify the values of a component from a specific entity in a thread-safe manner, we can do it like this: 
	entity->firstName.Do([](auto& firstName){
		firstName = "Paul";
	});
	entity->someData.Do([](auto& someData){
		someData.a = 44;
	});
	// This also ensures that the component exists before running the code, so you may run this with any entity without worrying if the component is present.
	// The 'Do' method also returns true if the entity has that component, false otherwise. 
	
	
	/////////////////////////////////
	// OneToMany component maps
	
	// If your needs require a single entity to be able to host a variable-size map of a certain type of component, you may define a component map
	// example using std::string_view as the map's key type in the second parameter of the macro
	V4D_ENTITY_DECLARE_COMPONENT_MAP(MyEntity, std::string_view, SomeDataStruct, someDataMap)
	
	// To use a component map, it is a little different than a standard OneToOne component.
	
	// You must first Add the component map to your entity like so: 
	entity->Add_someDataMap(); // note that it takes no parameter, but returns a reference to entity->someDataMap
	
	// You can add or access an instance of a component in the map using the operator[] and assign its value to your component (using its constructor) or any member of that component by following with the operator->
	entity->someDataMap["myKey"] = {...};
	entity->someDataMap["myKey"]->a = 6;
	cout << entity->someDataMap["myKey"]->a;
	
	// You may remove the entire component map like this and it will effectively erase all elements in it
	entity->Remove_someDataMap();
	
	// You can also erase specific keys from the map like this:
	entity->someDataMap.Erase("myKey");
	
	// You may loop through all components in the map related to a specific entity like with the Do() method above:
	entity->someDataMap.ForEach([](auto& someData){
		someData.a = 44;
	});
	
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
			uint32_t* componentIndexPtrInEntity;
			ComponentType component;
			template<typename...Args>
			ComponentTuple(int32_t i, uint32_t* cmpIdxPtr, Args&&...args) : entityInstanceIndex(i), componentIndexPtrInEntity(cmpIdxPtr), component(std::forward<Args>(args)...) {}
			template<typename T, int N>
			ComponentTuple(int32_t i, uint32_t* cmpIdxPtr, const T (&arr)[N]) : entityInstanceIndex(i), componentIndexPtrInEntity(cmpIdxPtr), component(arr) {}
		};
		std::vector<ComponentTuple> componentsList;
		template<typename...Args>
		void __Add__(int32_t entityInstanceIndex, void* componentIndexPtrInEntity, Args&&...args) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			componentsList.emplace_back(entityInstanceIndex, (uint32_t*)componentIndexPtrInEntity, std::forward<Args>(args)...);
			(*(int32_t*)componentIndexPtrInEntity) = componentsList.size() - 1;
		}
		template<typename List_T>
		void __Add__(int32_t entityInstanceIndex, void* componentIndexPtrInEntity, std::initializer_list<List_T>&& list) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			componentsList.emplace_back(entityInstanceIndex, (uint32_t*)componentIndexPtrInEntity, std::forward<std::initializer_list<List_T>>(list));
			(*(int32_t*)componentIndexPtrInEntity) = componentsList.size() - 1;
		}
		template<typename ArrayRef, int ArrayN>
		void __Add__(int32_t entityInstanceIndex, void* componentIndexPtrInEntity, const ArrayRef (&val)[ArrayN]) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			componentsList.emplace_back(entityInstanceIndex, (uint32_t*)componentIndexPtrInEntity, val);
			(*(int32_t*)componentIndexPtrInEntity) = componentsList.size() - 1;
		}
		void __Remove__(int32_t componentIndex) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			if (componentIndex != -1) {
				if ((size_t)componentIndex < componentsList.size()-1) {
					// Move the last element to the position we want to delete, then reassign the index to the last element's parent entity
					componentsList[componentIndex] = std::move(componentsList.back());
					if (componentsList[componentIndex].entityInstanceIndex != -1) {
						*componentsList[componentIndex].componentIndexPtrInEntity = componentIndex;
					} else {
						LOG_ERROR("component __Remove__ last component entity index is -1")
					}
				}
				if (componentsList.size() > 0) componentsList.pop_back();
			}
		}
		ComponentType* __Get__(int32_t componentIndex) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			if (componentIndex == -1) return nullptr;
			if ((size_t)componentIndex < componentsList.size()) {
				return &componentsList[componentIndex].component;
			}
			return nullptr;
		}
	public:
		void ForEach(std::function<void(int32_t& entityInstanceIndex, ComponentType& data)>&& func) {
			std::lock_guard lock(componentsMutex);
			for (auto&[entityInstanceIndex, cmpIdxPtr, data] : componentsList) {
				func(entityInstanceIndex, data);
			}
		}
		void ForEach_LockEntities(std::function<void(int32_t& entityInstanceIndex, ComponentType& data)>&& func) {
			auto entitiesLock = EntityClass::GetLock();
			std::lock_guard lock(componentsMutex);
			for (auto&[entityInstanceIndex, cmpIdxPtr, data] : componentsList) {
				func(entityInstanceIndex, data);
			}
		}
		size_t Count() const {
			std::lock_guard lock(componentsMutex);
			return componentsList.size();
		}
		bool Do(int32_t componentIndex, std::function<void(ComponentType& data)>&& func){
			std::lock_guard lock(componentsMutex);
			if (componentIndex == -1) return false;
			if ((size_t)componentIndex < componentsList.size()) {
				func(componentsList[componentIndex].component);
				return true;
			}
			return false;
		}
		class ComponentReferenceLocked {
		private: friend Component;
			std::unique_lock<std::recursive_mutex> lock;
			ComponentType* ptr;
			DELETE_COPY_MOVE_CONSTRUCTORS(ComponentReferenceLocked)
		public:
			ComponentReferenceLocked() : lock(), ptr(nullptr) {}
			ComponentReferenceLocked(std::unique_lock<std::recursive_mutex>& lock, ComponentType* ptr) : lock(std::move(lock)), ptr(ptr) {}
			operator bool() {return !!ptr;}
			template<typename T>
			auto operator= (T&& val) {
				*ptr = std::forward<T>(val);
				return std::forward<T>(val);
			}
			ComponentType* operator->() {return ptr;}
			void Unlock() {
				ptr = nullptr;
				lock = {};
			}
		};
		ComponentReferenceLocked Lock(int32_t componentIndex) {
			std::unique_lock<std::recursive_mutex> lock(componentsMutex);
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
		using EntityIndex_T = int32_t;\
		using ComponentIndex_T = int32_t;\
	private:\
		static std::optional<std::thread::id> destroyThreadId;\
		EntityIndex_T index;\
		bool active = true;\
		bool markedForDestruction = false;\
		ClassName(EntityIndex_T index);\
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
		static void CleanupOnThisThread();\
		static void Destroy(EntityIndex_T index);\
		static void Trim();\
		void Destroy();\
		static void ClearAll();\
		static size_t Count();\
		static size_t CountActive();\
		static std::unique_lock<std::recursive_mutex> GetLock();\
		static void ForEach(std::function<void(std::shared_ptr<ClassName>)>&& func);\
		static std::shared_ptr<ClassName> Get(EntityIndex_T entityInstanceIndex);\
		inline EntityIndex_T GetIndex() const {return index;};

// Used in .cpp files
#define V4D_ENTITY_DEFINE_CLASS(ClassName)\
	std::recursive_mutex ClassName::entityInstancesMutex {};\
	std::vector<std::shared_ptr<ClassName>> ClassName::entityInstances {};\
	std::optional<std::thread::id> ClassName::destroyThreadId = std::nullopt;\
	ClassName::ClassName(EntityIndex_T index) : index(index) {}\
	std::shared_ptr<ClassName> ClassName::Create() {\
		std::lock_guard lock(entityInstancesMutex);\
		size_t nbEntityInstances = entityInstances.size();\
		for (size_t i = 0; i < nbEntityInstances; ++i) {\
			if (!entityInstances[i]) return entityInstances[i] = std::shared_ptr<ClassName>(new ClassName(i));\
		}\
		return entityInstances.emplace_back(new ClassName(nbEntityInstances));\
	}\
	void ClassName::CleanupOnThisThread(){\
		std::lock_guard lock(entityInstancesMutex);\
		destroyThreadId = std::this_thread::get_id();\
		for (auto& entity : entityInstances) {\
			if (entity && entity->markedForDestruction) {\
				entity->index = -1;\
				entity.reset();\
			}\
		}\
		Trim();\
	}\
	void ClassName::Trim() {\
		std::lock_guard lock(entityInstancesMutex);\
		while (entityInstances.size() > 0 && !entityInstances[entityInstances.size()-1]) {\
			entityInstances.pop_back();\
		}\
	}\
	void ClassName::Destroy(EntityIndex_T index) {\
		std::lock_guard lock(entityInstancesMutex);\
		if (index != -1 && (size_t)index < entityInstances.size()) {\
			if (destroyThreadId.has_value() && destroyThreadId.value() != std::this_thread::get_id()) {\
				entityInstances[index]->markedForDestruction = true;\
			} else {\
				entityInstances[index]->index = -1;\
				entityInstances[index].reset();\
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
			if (entity && !entity->markedForDestruction) {\
				func(entity);\
			}\
		}\
	}\
	std::shared_ptr<ClassName> ClassName::Get(EntityIndex_T entityInstanceIndex) {\
		std::lock_guard lock(entityInstancesMutex);\
		if (entityInstanceIndex == -1 || (size_t)entityInstanceIndex >= entityInstances.size()) return nullptr;\
		return entityInstances[entityInstanceIndex];\
	}\
	void ClassName::ClearAll() {\
		std::lock_guard lock(entityInstancesMutex);\
		for (auto& entity : entityInstances) {\
			if (entity) entity->index = -1;\
		}\
		entityInstances.clear();\
	}\
	size_t ClassName::Count() {\
		std::lock_guard lock(entityInstancesMutex);\
		return entityInstances.size();\
	}\
	size_t ClassName::CountActive() {\
		size_t count = 0;\
		std::lock_guard lock(entityInstancesMutex);\
		for (auto& entity : entityInstances) {\
			if (entity && entity->index != -1) ++count;\
		}\
		return count;\
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
		ClassName::ComponentIndex_T index;\
		ComponentReference_ ## MemberName () : index(-1) {}\
		ComponentReference_ ## MemberName (ClassName::ComponentIndex_T componentIndex) : index(componentIndex) {}\
		~ComponentReference_ ## MemberName () {\
			std::lock_guard entitiesLock(ClassName::entityInstancesMutex);\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			if (index != -1) {\
				MemberName ## Components .__Remove__(index);\
			}\
		}\
		public:\
		operator bool() {return index != -1;}\
		template<typename T>\
		auto operator= (T&& val) {\
			if (index != -1) return MemberName ## Components .Lock(index).operator=(std::forward<T>(val));\
			return std::forward<T>(val);\
		}\
		bool Do(std::function<void(ComponentType&)>&& func){\
			if (index == -1) return false;\
			return MemberName ## Components .Do(index, std::forward<std::function<void(ComponentType&)>>(func));\
		}\
		v4d::data::EntityComponentSystem::Component<ClassName, ComponentType>::ComponentReferenceLocked Lock(){\
			return MemberName ## Components .Lock(index);\
		}\
	} MemberName;\
	static v4d::data::EntityComponentSystem::Component<ClassName, ComponentType> MemberName ## Components ;\
	/* Add_<component> */\
	template<typename...Args>\
	v4d::data::EntityComponentSystem::Component<ClassName, ComponentType>::ComponentReferenceLocked Add_ ## MemberName (Args&&...args) {\
		std::lock_guard entitiesLock(ClassName::entityInstancesMutex);\
		std::unique_lock<std::recursive_mutex> componentsLock(MemberName ## Components.componentsMutex);\
		if (MemberName.index == -1) MemberName ## Components .__Add__<Args...>(index, &MemberName.index, std::forward<Args>(args)...);\
		return {componentsLock, MemberName ## Components .__Get__(MemberName.index)};\
	}\
	template<typename List_T>\
	v4d::data::EntityComponentSystem::Component<ClassName, ComponentType>::ComponentReferenceLocked Add_ ## MemberName (std::initializer_list<List_T>&& list) {\
		std::lock_guard entitiesLock(ClassName::entityInstancesMutex);\
		std::unique_lock<std::recursive_mutex> componentsLock(MemberName ## Components.componentsMutex);\
		if (MemberName.index == -1) MemberName ## Components .__Add__<List_T>(index, &MemberName.index, std::forward<std::initializer_list<List_T>>(list));\
		return {componentsLock, MemberName ## Components .__Get__(MemberName.index)};\
	}\
	template<typename ArrayRef, int ArrayN>\
	v4d::data::EntityComponentSystem::Component<ClassName, ComponentType>::ComponentReferenceLocked Add_ ## MemberName (const ArrayRef (&val)[ArrayN]) {\
		std::lock_guard entitiesLock(ClassName::entityInstancesMutex);\
		std::unique_lock<std::recursive_mutex> componentsLock(MemberName ## Components.componentsMutex);\
		if (MemberName.index == -1) MemberName ## Components .__Add__<ArrayRef, ArrayN>(index, &MemberName.index, val);\
		return {componentsLock, MemberName ## Components .__Get__(MemberName.index)};\
	}\
	void Remove_ ## MemberName ();

// Used in .cpp files
#define V4D_ENTITY_DEFINE_COMPONENT(ClassName, ComponentType, MemberName) \
	v4d::data::EntityComponentSystem::Component<ClassName, ComponentType> ClassName::MemberName ## Components  {};\
	void ClassName::Remove_ ## MemberName () {\
		std::lock_guard entitiesLock(ClassName::entityInstancesMutex);\
		std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
		if (MemberName.index != -1) {\
			MemberName ## Components .__Remove__(MemberName.index);\
			MemberName.index = -1;\
		}\
	}


// Used in .h files
#define V4D_ENTITY_DECLARE_COMPONENT_MAP(ClassName, MapKey, ComponentType, MemberName) \
	class ComponentReferenceMap_ ## MemberName {\
		friend ClassName;\
		EntityIndex_T entityIndex;\
		std::unordered_map<MapKey, ClassName::ComponentIndex_T*> indices {};\
		ComponentReferenceMap_ ## MemberName () : entityIndex(-1) {}\
		ComponentReferenceMap_ ## MemberName (EntityIndex_T entityIndex) : entityIndex(entityIndex) {}\
		~ComponentReferenceMap_ ## MemberName () {\
			if (entityIndex == -1) return;\
			std::lock_guard entitiesLock(ClassName::entityInstancesMutex);\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			for (auto&[key, indexPtr] : indices) if (indexPtr) {\
				MemberName ## Components .__Remove__(*indexPtr);\
				delete indexPtr;\
			}\
		}\
		public:\
		operator bool() {return (entityIndex != -1);}\
		template<typename T>\
		v4d::data::EntityComponentSystem::Component<ClassName, ComponentType>::ComponentReferenceLocked operator[] (T&& key) {\
			if (entityIndex == -1) return {};\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			if (!indices[std::forward<T>(key)]) {\
				indices[std::forward<T>(key)] = new ClassName::ComponentIndex_T;\
				MemberName ## Components .__Add__(entityIndex, indices[std::forward<T>(key)]);\
			}\
			return MemberName ## Components .Lock(*indices[std::forward<T>(key)]);\
		}\
		bool ForEach(std::function<void(ComponentType&)>&& func){\
			if (entityIndex == -1) return false;\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			for (auto&[key, indexPtr] : indices) if(indexPtr) {\
				MemberName ## Components .Do(*indexPtr, std::forward<std::function<void(ComponentType&)>>(func));\
			}\
			return true;\
		}\
		size_t Count() const {\
			if (entityIndex == -1) return 0;\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			return indices.size();\
		}\
		void Erase(MapKey key) {\
			if (entityIndex == -1) return;\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			try {\
				ClassName::ComponentIndex_T* indexPtr = indices.at(key);\
				if (indexPtr) {\
					MemberName ## Components .__Remove__(*indexPtr);\
					delete indices[key];\
				}\
				indices.erase(key);\
			} catch(...){}\
		}\
	} MemberName;\
	static v4d::data::EntityComponentSystem::Component<ClassName, ComponentType> MemberName ## Components ;\
	/* Add_<component> */\
	ComponentReferenceMap_ ## MemberName & Add_ ## MemberName () {\
		MemberName.entityIndex = index;\
		return MemberName;\
	}\
	void Remove_ ## MemberName ();

// Used in .cpp files
#define V4D_ENTITY_DEFINE_COMPONENT_MAP(ClassName, MapKey, ComponentType, MemberName) \
	v4d::data::EntityComponentSystem::Component<ClassName, ComponentType> ClassName::MemberName ## Components  {};\
	void ClassName::Remove_ ## MemberName () {\
		if (MemberName.entityIndex == -1) return;\
		std::lock_guard entitiesLock(ClassName::entityInstancesMutex);\
		std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
		for (auto&[key, indexPtr] : MemberName.indices) if (indexPtr) {\
			MemberName ## Components .__Remove__(*indexPtr);\
			delete indexPtr;\
		}\
		MemberName.indices.clear();\
		MemberName.entityIndex = -1;\
	}

