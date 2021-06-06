#pragma once

#include <mutex>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>
#include <atomic>
#include <functional>
#include <thread>
#include <utility>

/**
 * V4D's ECS v1.3
 * Author: Olivier St-Laurent, December 2020 - June 2021
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
	MyEntity::Ptr entity = MyEntity::Create();
	
	// You cannot create any constructor for the entity as it is made opaque by this system.
	// However, if you want to construct things upon instantiation with custom arguments, you may have an operator()(...) overload and arguments you pass to Create(...) will be forwarded to it. Passing no argument to Create() will NOT call operator()().
	
	// The entity does not get destroyed when we leave the scope, it will live forever until we manually destroy them in one of three ways:
		
		// 1. Loop through each entity to determine which one to destroy and simply call ->Destroy() on the entity
		
			MyEntity::ForEach([](auto entity){
				entity->Destroy();
			});
			
		// 2. the ClearAll static member
		
			MyEntity::ClearAll();
			
		// 3. Call the Destroy static function with a given entity instance index
		
			MyEntity::Destroy(index);
			
		// All three ways are equivalent
		
	// Note: since they are shared pointers, if a reference exists somewhere else it will not actually get destroyed until all references are destroyed.
	// You may verify if the entity has been Destroy()ed using the ->IsDestroyed() method on an entity's remaining shared pointer.
	// Looping through entities or components will NOT include the ones that were Destroy()ed even if they still have a living reference somewhere.
	// You may define a destructor in your entity class and it will only be called when all references have actually been destroyed, not necessarily when the Destroy() function is called.
	
	// We can get the index of an entity like this:
	auto index = entity->GetIndex();
	// This index can then be used to fetch that entity again like this:
	auto entity = MyEntity::Get(index);
	// Entity indices will not change for a given entity as long as you don't destroy it. Then their indices will be reused by new entities, hence invalidated.
	// To verify if an entity has been destroyed, you may call IsDestroyed().
	
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
	MyEntity::firstNameComponents.ForEach([](auto entityInstanceIndex, auto& firstName, auto componentIndex){ // componentIndex arg is optional
		std::cout << firstName << std::endl;
	});
	// This will only run for existing components, not for entities that don't have that component added to it
	// All components of the same type are contiguous in memory for great CPU cache usage
	// Hence it is preferable to not use pointers as component types because it will defeat the purpose.
	// CAUTION: Watch out for deadlocks when looping through components
	//		If you intend to call MyEntity::Get(entityInstanceIndex) from the lambda in a component ForEach() function, you must use ForEach_LockEntities() or ForEach_Entity() instead.
	//		This will ensure that we always lock the entities around the components and not the other way around.
	//		For the same reason, from within a component ForEach(), you cannot do a ForEach() through another component type. You must loop through the entities themselves and fetch its components individually instead.
	//		Note that the first argument of the lambda passed to ForEach_Entity() is the shared pointer to the entity instead of its index.
	
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
	// you may otherwise use a lambda with a more complete set of args when needed
	entity->someDataMap.ForEach([](const auto& componentKey, auto& someData, auto componentIndex){
		someData.a = 44;
	});
	
	
	////////////////////////////////
	// Mapped Entities (using custom entity IDs instead of indices)
	
	// To be able to use predefined IDs as the key to your entity, simply declare your class using V4D_ENTITY_DECLARE_CLASS_MAP and V4D_ENTITY_DEFINE_CLASS_MAP, the same way as with the other similar macros.
	// Not to be confused with V4D_ENTITY_DECLARE_COMPONENT_MAP which are mapped components per entity and have nothing to do with mapped entities, they can be used interchangeably with one another.
	// The main difference between V4D_ENTITY_DECLARE_CLASS and V4D_ENTITY_DECLARE_CLASS_MAP is that entities are defined in a map instead of a vector.
	// This allows us to specify a custom ID for the instance, which can be used to get back the instance after creating it, instead of having to use an index based on the number of entity instances.
	// You may specify the ID as the first parameter in the Create() function. The remaining parameters will be passed on to the operator() function if applicable as explained previously.
	// It is also possible to have the system automatically increment to the next ID (by default starting at 0), simply by specifying -1 or by using no argument in the Create() function. 
	// You may then grab the generated ID using the same GetIndex() method, or a more appropriately named GetID() which returns the same thing.
	// You may also manually reserve/get a next ID to assign it afterwards manually, by using the ::NextID() function. 
	// It is also possible to specify which ID the system will use automatically next by passing an argument in the NextID() function. This will return the passed value and will not increment it just yet.
	// This mapped entity system is also useful even when you always auto increment the ID, in cases where you want to make sure that the deleted IDs never get reused as they do with the previous way using indices.
	// The two ways to declare entity classes are perfectly interchangeable except for the additional (1st) parameter in the Create() function and a few additional functions (GetID and NextID) present in the mapped version.
	// To refactor to the mapped version without breaking previous code, simply add a -1 as the first param in your Create() calls.
	
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace v4d::ECS {
	using EntityIndex_T = int64_t;
	using ComponentIndex_T = int64_t;
	
	// Components wrapper template
	template<typename EntityClass, typename ComponentType>
	class Component {
	friend EntityClass;
		mutable std::recursive_mutex componentsMutex;
		struct ComponentTuple {
			EntityIndex_T entityInstanceIndex;
			ComponentIndex_T* componentIndexPtrInEntity;
			ComponentType component;
			template<typename...Args>
			ComponentTuple(EntityIndex_T i, ComponentIndex_T* cmpIdxPtr, Args&&...args) : entityInstanceIndex(i), componentIndexPtrInEntity(cmpIdxPtr), component(std::forward<Args>(args)...) {}
			template<typename T, int N>
			ComponentTuple(EntityIndex_T i, ComponentIndex_T* cmpIdxPtr, const T (&arr)[N]) : entityInstanceIndex(i), componentIndexPtrInEntity(cmpIdxPtr), component(arr) {}
		};
		std::vector<ComponentTuple> componentsList;
		template<typename...Args>
		void __Add__(EntityIndex_T entityInstanceIndex, void* componentIndexPtrInEntity, Args&&...args) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			componentsList.emplace_back(entityInstanceIndex, (ComponentIndex_T*)componentIndexPtrInEntity, std::forward<Args>(args)...);
			(*(ComponentIndex_T*)componentIndexPtrInEntity) = componentsList.size() - 1;
		}
		template<typename List_T>
		void __Add__(EntityIndex_T entityInstanceIndex, void* componentIndexPtrInEntity, std::initializer_list<List_T>&& list) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			componentsList.emplace_back(entityInstanceIndex, (ComponentIndex_T*)componentIndexPtrInEntity, std::forward<std::initializer_list<List_T>>(list));
			(*(ComponentIndex_T*)componentIndexPtrInEntity) = componentsList.size() - 1;
		}
		template<typename ArrayRef, int ArrayN>
		void __Add__(EntityIndex_T entityInstanceIndex, void* componentIndexPtrInEntity, const ArrayRef (&val)[ArrayN]) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			componentsList.emplace_back(entityInstanceIndex, (ComponentIndex_T*)componentIndexPtrInEntity, val);
			(*(ComponentIndex_T*)componentIndexPtrInEntity) = componentsList.size() - 1;
		}
		void __Remove__(ComponentIndex_T componentIndex) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			if (componentIndex != -1) {
				if ((size_t)componentIndex < componentsList.size()-1) {
					// Move the last element to the position we want to delete, then reassign the index to the last element's parent entity
					componentsList[componentIndex] = std::move(componentsList.back());
					*componentsList[componentIndex].componentIndexPtrInEntity = componentIndex;
				}
				if (componentsList.size() > 0) componentsList.pop_back();
			}
		}
		ComponentType* __Get__(ComponentIndex_T componentIndex) {
			// std::lock_guard lock(componentsMutex); // always already locked in caller, and also locks Entities
			if (componentIndex == -1) return nullptr;
			if ((size_t)componentIndex < componentsList.size()) {
				return &componentsList[componentIndex].component;
			}
			return nullptr;
		}
	public:
		void ForEach(std::function<void(EntityIndex_T, ComponentType&)>&& func) {
			std::lock_guard lock(componentsMutex);
			for (auto&[entityInstanceIndex, cmpIdxPtr, data] : componentsList) {
				func(entityInstanceIndex, data);
			}
		}
		void ForEach_LockEntities(std::function<void(EntityIndex_T, ComponentType&)>&& func) {
			auto entitiesLock = EntityClass::GetLock();
			std::lock_guard lock(componentsMutex);
			for (auto&[entityInstanceIndex, cmpIdxPtr, data] : componentsList) {
				func(entityInstanceIndex, data);
			}
		}
		void ForEach_Entity(std::function<void(std::shared_ptr<EntityClass>&, ComponentType&)>&& func) {
			auto entitiesLock = EntityClass::GetLock();
			std::lock_guard lock(componentsMutex);
			for (auto&[entityInstanceIndex, cmpIdxPtr, data] : componentsList) {
				std::shared_ptr<EntityClass> entity = EntityClass::Get(entityInstanceIndex);
				if (entity) func(entity, data);
			}
		}
		void ForEach(std::function<void(EntityIndex_T, ComponentType&, ComponentIndex_T)>&& func) {
			std::lock_guard lock(componentsMutex);
			for (ComponentIndex_T componentIndex = 0; componentIndex < componentsList.size(); ++componentIndex) {
				auto&[entityInstanceIndex, cmpIdxPtr, data] = componentsList[componentIndex];
				func(entityInstanceIndex, data, componentIndex);
			}
		}
		void ForEach_LockEntities(std::function<void(EntityIndex_T, ComponentType&, ComponentIndex_T)>&& func) {
			auto entitiesLock = EntityClass::GetLock();
			std::lock_guard lock(componentsMutex);
			for (ComponentIndex_T componentIndex = 0; componentIndex < componentsList.size(); ++componentIndex) {
				auto&[entityInstanceIndex, cmpIdxPtr, data] = componentsList[componentIndex];
				func(entityInstanceIndex, data, componentIndex);
			}
		}
		void ForEach_Entity(std::function<void(std::shared_ptr<EntityClass>&, ComponentType&, ComponentIndex_T)>&& func) {
			auto entitiesLock = EntityClass::GetLock();
			std::lock_guard lock(componentsMutex);
			for (ComponentIndex_T componentIndex = 0; componentIndex < componentsList.size(); ++componentIndex) {
				auto&[entityInstanceIndex, cmpIdxPtr, data] = componentsList[componentIndex];
				std::shared_ptr<EntityClass> entity = EntityClass::Get(entityInstanceIndex);
				if (entity) func(entity, data, componentIndex);
			}
		}
		size_t Count() const {
			std::lock_guard lock(componentsMutex);
			return componentsList.size();
		}
		bool Do(ComponentIndex_T componentIndex, std::function<void(ComponentType& data)>&& func){
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
		ComponentReferenceLocked Lock(ComponentIndex_T componentIndex) {
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
	public: using Ptr = std::shared_ptr<ClassName>;\
	public: using WeakPtr = std::weak_ptr<ClassName>;\
	private:\
		static std::recursive_mutex _ecs_entityInstancesMutex;\
		static std::vector<Ptr> _ecs_entityInstances;\
	private:\
		static std::optional<std::thread::id> _ecs_destroyThreadId;\
		v4d::ECS::EntityIndex_T _ecs_index;\
		bool _ecs_markedForDestruction = false;\
		bool _ecs_destroyed = false;\
		ClassName(v4d::ECS::EntityIndex_T);\
	public:\
		static Ptr Create();\
		template<typename...Args>\
		static Ptr Create(Args&&...args) {\
			std::lock_guard lock(_ecs_entityInstancesMutex);\
			size_t nbEntityInstances = _ecs_entityInstances.size();\
			for (size_t i = 0; i < nbEntityInstances; ++i) {\
				if (!_ecs_entityInstances[i]) {\
					auto* e = new ClassName(i);\
					(*e)(std::forward<Args>(args)...);\
					return _ecs_entityInstances[i] = Ptr(e);\
				}\
			}\
			auto* e = new ClassName(nbEntityInstances);\
			(*e)(std::forward<Args>(args)...);\
			return _ecs_entityInstances.emplace_back(e);\
		}\
		static void CleanupOnThisThread();\
		static void Destroy(v4d::ECS::EntityIndex_T);\
		static void Trim();\
		void Destroy();\
		static void ClearAll();\
		static size_t Count();\
		static size_t CountActive();\
		static std::unique_lock<std::recursive_mutex> GetLock();\
		static void ForEach(std::function<void(Ptr)>&& func);\
		static Ptr Get(v4d::ECS::EntityIndex_T entityInstanceIndex);\
		inline v4d::ECS::EntityIndex_T GetIndex() const {return _ecs_index;};\
		inline bool IsDestroyed() const {return _ecs_destroyed;}

// Used in .cpp files
#define V4D_ENTITY_DEFINE_CLASS(ClassName)\
	std::recursive_mutex ClassName::_ecs_entityInstancesMutex {};\
	std::vector<ClassName::Ptr> ClassName::_ecs_entityInstances {};\
	std::optional<std::thread::id> ClassName::_ecs_destroyThreadId = std::nullopt;\
	ClassName::ClassName(v4d::ECS::EntityIndex_T index) : _ecs_index(index) {}\
	ClassName::Ptr ClassName::Create() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		size_t nbEntityInstances = _ecs_entityInstances.size();\
		for (size_t i = 0; i < nbEntityInstances; ++i) {\
			if (!_ecs_entityInstances[i]) return _ecs_entityInstances[i] = ClassName::Ptr(new ClassName(i));\
		}\
		return _ecs_entityInstances.emplace_back(new ClassName(nbEntityInstances));\
	}\
	void ClassName::CleanupOnThisThread(){\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		_ecs_destroyThreadId = std::this_thread::get_id();\
		for (auto& entity : _ecs_entityInstances) {\
			if (entity && entity->_ecs_markedForDestruction) {\
				entity->_ecs_destroyed = true;\
				entity.reset();\
			}\
		}\
		Trim();\
	}\
	void ClassName::Trim() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		while (_ecs_entityInstances.size() > 0 && !_ecs_entityInstances[_ecs_entityInstances.size()-1]) {\
			_ecs_entityInstances.pop_back();\
		}\
	}\
	void ClassName::Destroy(v4d::ECS::EntityIndex_T index) {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		if (index >= 0 && (size_t)index < _ecs_entityInstances.size()) {\
			if (_ecs_destroyThreadId.has_value() && _ecs_destroyThreadId.value() != std::this_thread::get_id()) {\
				_ecs_entityInstances[index]->_ecs_markedForDestruction = true;\
			} else {\
				_ecs_entityInstances[index]->_ecs_destroyed = true;\
				_ecs_entityInstances[index].reset();\
			}\
		}\
	}\
	void ClassName::Destroy() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		Destroy(_ecs_index);\
	}\
	void ClassName::ForEach(std::function<void(ClassName::Ptr)>&& func) {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		for (auto& entity : _ecs_entityInstances) {\
			if (entity && !entity->_ecs_markedForDestruction) {\
				func(entity);\
			}\
		}\
	}\
	ClassName::Ptr ClassName::Get(v4d::ECS::EntityIndex_T entityInstanceIndex) {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		if (entityInstanceIndex < 0 || (size_t)entityInstanceIndex >= _ecs_entityInstances.size()) return nullptr;\
		return _ecs_entityInstances[entityInstanceIndex];\
	}\
	void ClassName::ClearAll() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		for (auto& entity : _ecs_entityInstances) {\
			if (entity) entity->_ecs_destroyed = true;\
		}\
		_ecs_entityInstances.clear();\
	}\
	size_t ClassName::Count() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		return _ecs_entityInstances.size();\
	}\
	size_t ClassName::CountActive() {\
		size_t count = 0;\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		for (auto& entity : _ecs_entityInstances) {\
			if (entity && !entity->_ecs_markedForDestruction) ++count;\
		}\
		return count;\
	}\
	std::unique_lock<std::recursive_mutex> ClassName::GetLock() {\
		return std::unique_lock<std::recursive_mutex>{_ecs_entityInstancesMutex};\
	}


// Used in .h files
#define V4D_ENTITY_DECLARE_CLASS_MAP(ClassName)\
	public: using Ptr = std::shared_ptr<ClassName>;\
	public: using WeakPtr = std::weak_ptr<ClassName>;\
	private:\
		static std::recursive_mutex _ecs_entityInstancesMutex;\
		static std::unordered_map<v4d::ECS::EntityIndex_T, Ptr> _ecs_entityInstances;\
	private:\
		static std::optional<std::thread::id> _ecs_destroyThreadId;\
		v4d::ECS::EntityIndex_T _ecs_index;\
		bool _ecs_markedForDestruction = false;\
		bool _ecs_destroyed = false;\
		ClassName(v4d::ECS::EntityIndex_T);\
	public:\
		static v4d::ECS::EntityIndex_T NextID(v4d::ECS::EntityIndex_T setNext = -1);\
		static Ptr Create(v4d::ECS::EntityIndex_T id = -1);\
		template<typename...Args>\
		static Ptr Create(v4d::ECS::EntityIndex_T id, Args&&...args) {\
			std::lock_guard lock(_ecs_entityInstancesMutex);\
			if (id < 0) id = NextID();\
			auto* e = new ClassName(id);\
			(*e)(std::forward<Args>(args)...);\
			return _ecs_entityInstances[id] = Ptr(e);\
		}\
		static void CleanupOnThisThread();\
		static void Destroy(v4d::ECS::EntityIndex_T);\
		static void Trim();\
		void Destroy();\
		static void ClearAll();\
		static size_t Count();\
		static size_t CountActive();\
		static std::unique_lock<std::recursive_mutex> GetLock();\
		static void ForEach(std::function<void(Ptr)>&& func);\
		static Ptr Get(v4d::ECS::EntityIndex_T id);\
		inline v4d::ECS::EntityIndex_T GetIndex() const {return _ecs_index;};\
		inline v4d::ECS::EntityIndex_T GetID() const {return _ecs_index;};\
		inline bool IsDestroyed() const {return _ecs_destroyed;}

// Used in .cpp files
#define V4D_ENTITY_DEFINE_CLASS_MAP(ClassName)\
	std::recursive_mutex ClassName::_ecs_entityInstancesMutex {};\
	std::unordered_map<v4d::ECS::EntityIndex_T, ClassName::Ptr> ClassName::_ecs_entityInstances;\
	std::optional<std::thread::id> ClassName::_ecs_destroyThreadId = std::nullopt;\
	ClassName::ClassName(v4d::ECS::EntityIndex_T index) : _ecs_index(index) {}\
	v4d::ECS::EntityIndex_T ClassName::NextID(v4d::ECS::EntityIndex_T setNext){\
		static std::atomic<v4d::ECS::EntityIndex_T> nextID = 0;\
		if (setNext >= 0) return nextID = setNext;\
		return nextID++;\
	}\
	ClassName::Ptr ClassName::Create(v4d::ECS::EntityIndex_T id) {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		if (id < 0) id = NextID();\
		return _ecs_entityInstances[id] = ClassName::Ptr(new ClassName(id));\
	}\
	void ClassName::CleanupOnThisThread(){\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		_ecs_destroyThreadId = std::this_thread::get_id();\
		for (auto&[id,entity] : _ecs_entityInstances) {\
			if (entity && entity->_ecs_markedForDestruction) {\
				entity->_ecs_destroyed = true;\
				entity.reset();\
			}\
		}\
		Trim();\
	}\
	void ClassName::Trim() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		for (auto it = _ecs_entityInstances.begin(); it != _ecs_entityInstances.end(); ) {\
			auto&[id, entity] = *it;\
			if (!entity) it = _ecs_entityInstances.erase(it);\
			else ++it;\
		}\
	}\
	void ClassName::Destroy(v4d::ECS::EntityIndex_T id) {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		if (id >= 0 && _ecs_entityInstances.count(id) > 0) {\
			if (_ecs_destroyThreadId.has_value() && _ecs_destroyThreadId.value() != std::this_thread::get_id()) {\
				_ecs_entityInstances[id]->_ecs_markedForDestruction = true;\
			} else {\
				_ecs_entityInstances[id]->_ecs_destroyed = true;\
				_ecs_entityInstances[id].reset();\
			}\
		}\
	}\
	void ClassName::Destroy() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		Destroy(_ecs_index);\
	}\
	void ClassName::ForEach(std::function<void(ClassName::Ptr)>&& func) {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		for (auto&[id,entity] : _ecs_entityInstances) {\
			if (entity && !entity->_ecs_markedForDestruction) {\
				func(entity);\
			}\
		}\
	}\
	ClassName::Ptr ClassName::Get(v4d::ECS::EntityIndex_T id) {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		if (id < 0 || _ecs_entityInstances.count(id) == 0) return nullptr;\
		return _ecs_entityInstances[id];\
	}\
	void ClassName::ClearAll() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		for (auto&[id,entity] : _ecs_entityInstances) {\
			if (entity) entity->_ecs_destroyed = true;\
		}\
		_ecs_entityInstances.clear();\
	}\
	size_t ClassName::Count() {\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		return _ecs_entityInstances.size();\
	}\
	size_t ClassName::CountActive() {\
		size_t count = 0;\
		std::lock_guard lock(_ecs_entityInstancesMutex);\
		for (auto&[id,entity] : _ecs_entityInstances) {\
			if (entity && !entity->_ecs_markedForDestruction) ++count;\
		}\
		return count;\
	}\
	std::unique_lock<std::recursive_mutex> ClassName::GetLock() {\
		return std::unique_lock<std::recursive_mutex>{_ecs_entityInstancesMutex};\
	}


// Used in .h files
#define V4D_ENTITY_DECLARE_COMPONENT(ClassName, ComponentType, MemberName) \
	class ComponentReference_ ## MemberName {\
		friend ClassName;\
		v4d::ECS::ComponentIndex_T index;\
		ComponentReference_ ## MemberName () : index(-1) {}\
		ComponentReference_ ## MemberName (v4d::ECS::ComponentIndex_T componentIndex) : index(componentIndex) {}\
		~ComponentReference_ ## MemberName () {\
			std::lock_guard entitiesLock(ClassName::_ecs_entityInstancesMutex);\
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
		v4d::ECS::Component<ClassName, ComponentType>::ComponentReferenceLocked Lock(){\
			return MemberName ## Components .Lock(index);\
		}\
	} MemberName;\
	static v4d::ECS::Component<ClassName, ComponentType> MemberName ## Components ;\
	/* Add_<component> */\
	template<typename...Args>\
	v4d::ECS::Component<ClassName, ComponentType>::ComponentReferenceLocked Add_ ## MemberName (Args&&...args) {\
		std::lock_guard entitiesLock(ClassName::_ecs_entityInstancesMutex);\
		std::unique_lock<std::recursive_mutex> componentsLock(MemberName ## Components.componentsMutex);\
		if (MemberName.index == -1) MemberName ## Components .__Add__<Args...>(_ecs_index, &MemberName.index, std::forward<Args>(args)...);\
		return {componentsLock, MemberName ## Components .__Get__(MemberName.index)};\
	}\
	template<typename List_T>\
	v4d::ECS::Component<ClassName, ComponentType>::ComponentReferenceLocked Add_ ## MemberName (std::initializer_list<List_T>&& list) {\
		std::lock_guard entitiesLock(ClassName::_ecs_entityInstancesMutex);\
		std::unique_lock<std::recursive_mutex> componentsLock(MemberName ## Components.componentsMutex);\
		if (MemberName.index == -1) MemberName ## Components .__Add__<List_T>(_ecs_index, &MemberName.index, std::forward<std::initializer_list<List_T>>(list));\
		return {componentsLock, MemberName ## Components .__Get__(MemberName.index)};\
	}\
	template<typename ArrayRef, int ArrayN>\
	v4d::ECS::Component<ClassName, ComponentType>::ComponentReferenceLocked Add_ ## MemberName (const ArrayRef (&val)[ArrayN]) {\
		std::lock_guard entitiesLock(ClassName::_ecs_entityInstancesMutex);\
		std::unique_lock<std::recursive_mutex> componentsLock(MemberName ## Components.componentsMutex);\
		if (MemberName.index == -1) MemberName ## Components .__Add__<ArrayRef, ArrayN>(_ecs_index, &MemberName.index, val);\
		return {componentsLock, MemberName ## Components .__Get__(MemberName.index)};\
	}\
	void Remove_ ## MemberName ();

// Used in .cpp files
#define V4D_ENTITY_DEFINE_COMPONENT(ClassName, ComponentType, MemberName) \
	v4d::ECS::Component<ClassName, ComponentType> ClassName::MemberName ## Components  {};\
	void ClassName::Remove_ ## MemberName () {\
		std::lock_guard entitiesLock(ClassName::_ecs_entityInstancesMutex);\
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
		v4d::ECS::EntityIndex_T entityIndex;\
		std::unordered_map<MapKey, v4d::ECS::ComponentIndex_T*> indices {};\
		ComponentReferenceMap_ ## MemberName () : entityIndex(-1) {}\
		ComponentReferenceMap_ ## MemberName (v4d::ECS::EntityIndex_T entityIndex) : entityIndex(entityIndex) {}\
		~ComponentReferenceMap_ ## MemberName () {\
			if (entityIndex == -1) return;\
			std::lock_guard entitiesLock(ClassName::_ecs_entityInstancesMutex);\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			for (auto&[key, indexPtr] : indices) if (indexPtr) {\
				MemberName ## Components .__Remove__(*indexPtr);\
				delete indexPtr;\
			}\
		}\
		public:\
		operator bool() {return (entityIndex != -1);}\
		template<typename T>\
		v4d::ECS::Component<ClassName, ComponentType>::ComponentReferenceLocked operator[] (T&& key) {\
			if (entityIndex == -1) return {};\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			if (!indices[std::forward<T>(key)]) {\
				indices[std::forward<T>(key)] = new v4d::ECS::ComponentIndex_T;\
				MemberName ## Components .__Add__(entityIndex, indices[std::forward<T>(key)]);\
			}\
			return MemberName ## Components .Lock(*indices[std::forward<T>(key)]);\
		}\
		bool ForEach(std::function<void(ComponentType&)>&& func){\
			if (entityIndex == -1) return false;\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			for (auto&[key, indexPtr] : indices) if(indexPtr) {\
				if (auto* componentPtr = MemberName ## Components .__Get__(*indexPtr); componentPtr) {\
					func(*componentPtr);\
				}\
			}\
			return true;\
		}\
		bool ForEach(std::function<void(const MapKey&, ComponentType&, v4d::ECS::ComponentIndex_T)>&& func){\
			if (entityIndex == -1) return false;\
			std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
			for (auto&[key, indexPtr] : indices) if(indexPtr) {\
				if (auto* componentPtr = MemberName ## Components .__Get__(*indexPtr); componentPtr) {\
					func(key, *componentPtr, *indexPtr);\
				}\
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
				v4d::ECS::ComponentIndex_T* indexPtr = indices.at(key);\
				if (indexPtr) {\
					MemberName ## Components .__Remove__(*indexPtr);\
					delete indices[key];\
				}\
				indices.erase(key);\
			} catch(...){}\
		}\
	} MemberName;\
	static v4d::ECS::Component<ClassName, ComponentType> MemberName ## Components ;\
	/* Add_<component> */\
	ComponentReferenceMap_ ## MemberName & Add_ ## MemberName () {\
		MemberName.entityIndex = _ecs_index;\
		return MemberName;\
	}\
	void Remove_ ## MemberName ();

// Used in .cpp files
#define V4D_ENTITY_DEFINE_COMPONENT_MAP(ClassName, MapKey, ComponentType, MemberName) \
	v4d::ECS::Component<ClassName, ComponentType> ClassName::MemberName ## Components  {};\
	void ClassName::Remove_ ## MemberName () {\
		if (MemberName.entityIndex == -1) return;\
		std::lock_guard entitiesLock(ClassName::_ecs_entityInstancesMutex);\
		std::lock_guard componentsLock(MemberName ## Components.componentsMutex);\
		for (auto&[key, indexPtr] : MemberName.indices) if (indexPtr) {\
			MemberName ## Components .__Remove__(*indexPtr);\
			delete indexPtr;\
		}\
		MemberName.indices.clear();\
		MemberName.entityIndex = -1;\
	}

