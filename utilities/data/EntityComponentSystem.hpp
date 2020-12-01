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
	
	// The entity does not get destroyed when we leave the scope, it will live forever until we manually destroy them in one of three ways:
		
		// 1. Loop through each entity to determine which one to destroy and simply assign it to nullptr to destroy it
		
			MyEntity::ForEach([](auto& entity){
				entity = nullptr;
			});
			
		// 2. the ClearAll static member
		
			MyEntity::ClearAll();
			
		// 3. Call Destroy with a given entity instance index
		
			MyEntity::Destroy(index);
			
		// All three ways are equivalent
		
	// Note: since they are shared pointers, if a reference exists somewhere else it will not get destroyed until all references are destroyed, but its index WILL be invalidated.
	// Also, make sure that all individual components have everything they need to correctly free the memory in their respective destructors
	
	// We can get the index of an entity like this:
	uint32_t index = entity->GetIndex();
	// This index can then be used to fetch that entity again like this:
	auto entity = MyEntity::Get(index);
	// Entity indices will not change for a given entity as long as you don't destroy it. Then their indices will be reused by new entities, hence invalidated.
	
	// add a 'firstName' component which is defined above as a std::string, and pass "Bob" in its constructor
	entity->Add_firstName("Bob");
	// The component is constructed in-place with the given arguments. 
	// If the component had already been added to that entity, it will be reconstructed and replace the old one which will be destroyed.
	
	// we may also remove the component. This actually removes the component, it doesn't just assign it to "".
	entity->Remove_firstName();
	// If the component was already not present, this does nothing
	
	// We can loop through all firstNames like this:
	MyEntity::firstNameComponents.ForEach([](uint32_t entityInstanceIndex, auto& firstName){
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

template<typename EntityClass, typename ComponentType>
class Component {
friend EntityClass;
	std::mutex componentsMutex;
	std::vector<std::tuple<int32_t/*entityInstanceIndex*/, ComponentType>> componentsList;
	template<typename...Args>
	int32_t Add(int32_t entityInstanceIndex, Args&&...args) { 
		std::lock_guard lock(componentsMutex);
		componentsList.push_back({entityInstanceIndex, std::forward<ComponentType>(ComponentType(std::forward<Args>(args)...))});
		return componentsList.size() - 1;
	}
	int32_t Remove(int32_t componentIndex) {
		int32_t replacementInstanceIndex = -1;
		std::lock_guard lock(componentsMutex);
		if (componentIndex < componentsList.size()) {
			auto& data = componentsList[componentsList.size() - 1];
			componentsList[componentIndex] = data;
			replacementInstanceIndex = std::get<0>(data);
		}
		componentsList.pop_back();
		return replacementInstanceIndex;
	}
	ComponentType* Get(int32_t componentIndex) {
		if (componentIndex == -1) return nullptr;
		std::lock_guard lock(componentsMutex);
		if (componentIndex < componentsList.size()) {
			return &std::get<1>(componentsList[componentIndex]);
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
	bool Do(int32_t componentIndex, std::function<void(ComponentType& data)>&& func){
		if (componentIndex == -1) return false;
		std::lock_guard lock(componentsMutex);
		if (componentIndex < componentsList.size()) {
			func(std::get<1>(componentsList[componentIndex]));
			return true;
		}
		return false;
	}
};


/////////////////////////////////////////////////////////
// MACROS for custom entity class definitions

// Used in .h files
#define V4D_ENTITY_DECLARE_CLASS(ClassName)\
	private:\
		static std::mutex entityInstancesMutex;\
		static std::vector<std::shared_ptr<ClassName>> entityInstances;\
		uint32_t index;\
		ClassName(uint32_t index);\
	public:\
		static std::shared_ptr<ClassName> Create();\
		static void Destroy(uint32_t index);\
		static void ClearAll();\
		static void ForEach(std::function<void(std::shared_ptr<ClassName>&)>&& func);\
		static std::shared_ptr<ClassName> Get(uint32_t entityInstanceIndex);\
		inline uint32_t GetIndex() const {return index;};

// Used in .cpp files
#define V4D_ENTITY_DEFINE_CLASS(ClassName)\
	std::mutex ClassName::entityInstancesMutex {};\
	std::vector<std::shared_ptr<ClassName>> ClassName::entityInstances {};\
	ClassName::ClassName(uint32_t index) : index(index) {}\
	std::shared_ptr<ClassName> ClassName::Create() {\
		std::lock_guard lock(entityInstancesMutex);\
		size_t nbEntityInstances = entityInstances.size();\
		for (size_t i = 0; i < nbEntityInstances; ++i) {\
			if (!entityInstances[i]) return entityInstances[i] = std::shared_ptr<ClassName>(new ClassName(i));\
		}\
		return entityInstances.emplace_back(std::shared_ptr<ClassName>(new ClassName(nbEntityInstances)));\
	}\
	void ClassName::Destroy(uint32_t index) {\
		std::lock_guard lock(entityInstancesMutex);\
		if (index < entityInstances.size()) {\
			entityInstances[index] = nullptr;\
		}\
	}\
	void ClassName::ForEach(std::function<void(std::shared_ptr<ClassName>&)>&& func) {\
		std::lock_guard lock(entityInstancesMutex);\
		for (auto& entity : entityInstances) {\
			if (entity) {\
				func(entity);\
			}\
		}\
	}\
	std::shared_ptr<ClassName> ClassName::Get(uint32_t entityInstanceIndex) {\
		std::lock_guard lock(entityInstancesMutex);\
		if (entityInstanceIndex == -1 || entityInstanceIndex >= entityInstances.size()) return nullptr;\
		return entityInstances[entityInstanceIndex];\
	}\
	void ClassName::ClearAll() {\
		std::lock_guard lock(entityInstancesMutex);\
		entityInstances.clear();\
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
			if (index != -1) MemberName ## Components .Remove(index);\
		}\
		ComponentType* Get() {\
			if (index == -1) return nullptr;\
			return MemberName ## Components .Get(index);\
		}\
		public:\
		operator bool() {return index != -1 && !!Get();}\
		ComponentType* operator -> () {return Get();}\
		bool Do(std::function<void(ComponentType&)>&& func){\
			if (index == -1) return false;\
			return MemberName ## Components .Do(index, std::forward<std::function<void(ComponentType&)>>(func));\
		}\
	} MemberName;\
	static Component<ClassName, ComponentType> MemberName ## Components ;\
	template<typename...Args>\
	ClassName* Add_ ## MemberName (Args&&...args) {\
		std::lock_guard lock(ClassName::entityInstancesMutex);\
		if (MemberName.index == -1)\
			MemberName.index = MemberName ## Components .Add(index, std::forward<Args>(args)...);\
		else \
			*MemberName.Get() = ComponentType(std::forward<Args>(args)...);\
		return this;\
	}\
	void Remove_ ## MemberName ();

// Used in .cpp files
#define V4D_ENTITY_DEFINE_COMPONENT(ClassName, ComponentType, MemberName) \
	Component<ClassName, ComponentType> ClassName::MemberName ## Components  {};\
	void ClassName::Remove_ ## MemberName () {\
		std::lock_guard lock(ClassName::entityInstancesMutex);\
		if (MemberName.index != -1) {\
			auto replacementInstanceIndex = MemberName ## Components .Remove(MemberName.index);\
			if (replacementInstanceIndex != -1 && ClassName::entityInstances[replacementInstanceIndex]) {\
				ClassName::entityInstances[replacementInstanceIndex]-> MemberName.index = MemberName.index ;\
				MemberName.index = -1;\
			}\
		}\
	}

