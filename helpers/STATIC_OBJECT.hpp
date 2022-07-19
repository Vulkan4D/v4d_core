/**
 * Static Objects helper v1.3 - 2022-05-20
 * Author: Olivier St-Laurent
 * 
 * This helper will add to a class the ability to loop through all instances of it and keep track of them, in a thread-safe manner.
 * (Instances are NOT contiguous in memory, hence not cache friendly, so this helper is most useful for code clarity, not performance)
 * 
 * It consists simply of the macros STATIC_OBJECT_[H|CPP|EXT](), and using them will basically add the following reserved methods:
 * 		* Self() // returns the shared_ptr of this static object, to be called within another member method (except the constructor/destructor).
 * 		* SelfWeak() // returns the weak_ptr of this static object, to be called within another member method (except the constructor/destructor).
 * 		* Create(...) // static method used to instantiate the object, it's the public version of the constructor and takes any argument list compatible with the constructors you define
 * 		* GetOrCreate(id, ...) // static method used to get or instantiate the object if it doesn't exist, it takes any argument list compatible with the constructors you define after the id. This may return nullptr if the object exists but is not of the correct type.
 * 		* Destroy(id) // static method that effectively removes the object from the global instance list, using either its id or the shared pointer. If we want to destroy an instance from inside a ForEach, we should call reset() on the ptr instead, or just assign it to nullptr.
 * 		* ForEach(func<void(Ptr&)>) // static method that allows looping through all instances of a static object using a lambda
 * 		* ClearUnused() // static method that removes all objects that are unused from the global instance list
 * 		* ClearAll() // static method that removes all objects from the global instance list
 * 		* Count() // static method that returns the current number of instances of this type
 * 		* GetID() // static method that returns the current id of this instance
 * 		* Get(id) // static method that returns the shared pointer of the object with the specified id (or null if it doesn't exist or if it's not the correct type)
 * 		* Exists(id) // static method that returns true if the object exists or false if it doesn't exist or if it's not the correct type
 * 		* GetLock() // static method that returns a lock on all instances of this type for Add/Remove/ForEach operations (but not Create/Destroy)
 * 
 * Usage:
 * 
 * 	// Test.h
 * 		class Test {
 * 			STATIC_OBJECT_H(Test) // Define this class as a static object here.
 * 			Test() {} // You may define any number of constructors with any number of arguments, with private/protected accessibility
 * 			Test(Id id) : id(id) {} // You may also define constructors that specify a custom id for this object instead of using the next available id.
 * 			// Any other private/protected members here
 * 			// You may call Self() from within a member method (except the constructors/destructor) to get the shared pointer.
 * 		public:
 * 			virtual ~Test() {} // Destructor must have public accessibility. Also, the destructor must be virtual if we are planning on extending this class.
 * 			// Any other public members here
 * 		};
 * 
 * 	// Test.cpp
 * 		STATIC_OBJECT_CPP(Test)
 * 
 * 	// main.cpp
 * 		#include "Test.h"
 * 		int main() {
 * 			Test::Ptr myTestObject = Test::Create(); // you may pass any argument list that are compatible with your constructors in Create()
 * 			// myTestObject is a shared pointer to your newly created object
 * 			//...
 * 			Test::ForEach([](Test::Ptr& test){ // Loops through all existing and non-destroyed references to objects of type Test
 * 				// do stuff here with test (it's a shared pointer to your object)
 * 			});
 * 			//...
 * 			Test::Destroy(myTestObject) // you may destroy using the object's shared ptr or its id. It will actually be destroyed only when all of its references are cleared.
 * 		}
 * 
 * 
 * // Inheritance
 * 		// It is also possible to extend a static object class, while they all share the same global instance list as the parent-most static object (the class that has the STATIC_OBJECT_H macro defined)
 * 		// To do this, child classes must simply have the macro STATIC_OBJECT_EXT(ChildClass, ParentMostClass)
 * 		// and define a constructor that invokes a parent constructor
 * 		// The Parent-most static object's destructor must be virtual
 * 
 * 		class Child : public Parent {
 * 			STATIC_OBJECT_EXT(Child, Parent)
 * 			Child() : Parent() {} // Again, you may have any number of arguments that are forwarded from when you call Child::Create()
 * 		  public: // Again, destructor must have public accessibility
 * 			~Child() {}
 * 		};
 * 
 *
 *	// Special usage:
 *		// Compiling a shared library (DLL) on windows to store the global object list:
 *		// you may optionally add a 2nd argument to the macro STATIC_OBJECT_H with either __declspec(dllexport) or __declspec(dllimport)
 *		// depending on whether you are compiling the shared library or the executable, respectively.
 *
 */

#pragma once

#include <mutex>
#include <unordered_map>
#include <memory>
#include <atomic>

#define STATIC_OBJECT_H(ClassName, .../* may optionally have a 2nd argument with either __declspec(dllexport) or __declspec(dllimport) */) \
	public:\
		using Ptr = std::shared_ptr<ClassName>;\
		using WeakPtr = std::weak_ptr<ClassName>;\
		using Id = int64_t;\
	private:\
		static std::unordered_map<Id, Ptr> _sharedObjects;\
		static std::recursive_mutex _sharedObjectsMutex;\
		WeakPtr _self;\
		class __VA_ARGS__ AutoIncrementID {\
			ClassName::Id _id;\
		public:\
			AutoIncrementID(ClassName::Id = -1);\
			operator ClassName::Id() const {return _id;}\
		};\
		const AutoIncrementID id;\
	protected:\
		Ptr Self() {return _self.lock();}\
		WeakPtr SelfWeak() {return _self;}\
	public:\
		Id GetID() const {return id;}\
		template<class C = ClassName>\
		static auto Get(Id id) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			if (_sharedObjects.contains(id)) {\
				return std::dynamic_pointer_cast<C>(_sharedObjects.at(id));\
			}\
			return typename C::Ptr{};\
		}\
		template<class C = ClassName>\
		static bool Exists(Id id) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			if (_sharedObjects.contains(id)) {\
				return !!std::dynamic_pointer_cast<C>(_sharedObjects.at(id));\
			}\
			return false;\
		}\
		template<class C = ClassName, typename...Args>\
		static auto Create(Args&&...args) {\
			typename C::Ptr sharedPtr(new C(std::forward<Args>(args)...));\
			sharedPtr->_self = sharedPtr;\
			std::lock_guard lock(_sharedObjectsMutex);\
			_sharedObjects.emplace(sharedPtr->GetID(), sharedPtr);\
			return sharedPtr;\
		}\
		template<class C = ClassName, typename...Args>\
		static auto GetOrCreate(Id id, Args&&...args) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			if (_sharedObjects.contains(id) && _sharedObjects.at(id)) {\
				return std::dynamic_pointer_cast<C>(_sharedObjects.at(id));\
			}\
			typename C::Ptr sharedPtr(new C(id, std::forward<Args>(args)...));\
			sharedPtr->_self = sharedPtr;\
			_sharedObjects.emplace(id, sharedPtr);\
			return sharedPtr;\
		}\
		static void ForEach(std::function<void(Ptr&)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			for (auto it = _sharedObjects.begin(); it != _sharedObjects.end();) {\
				if (it->second) func(it->second);\
				if (!it->second) {\
					it = _sharedObjects.erase(it);\
				} else ++it;\
			}\
		}\
		static size_t Count() {\
			std::lock_guard lock(_sharedObjectsMutex);\
			return _sharedObjects.size();\
		}\
		static void ClearUnused() {\
			std::lock_guard lock(_sharedObjectsMutex);\
			for (auto it = _sharedObjects.begin(); it != _sharedObjects.end();) {\
				if (!it->second || it->second.unique()) it = _sharedObjects.erase(it);\
				else ++it;\
			}\
		}\
		static void ClearAll() {\
			std::lock_guard lock(_sharedObjectsMutex);\
			_sharedObjects.clear();\
		}\
		static void Destroy(Id id) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			if (_sharedObjects.contains(id)) {\
				_sharedObjects.at(id).reset();\
			}\
		}\
		bool IsDestroyed() {\
			std::lock_guard lock(_sharedObjectsMutex);\
			return !_sharedObjects.contains(id);\
		}\
		static void Destroy(Ptr& ptr) {\
			Destroy(ptr->GetID());\
		}\
		static std::unique_lock<std::recursive_mutex> GetLock() {\
			return std::unique_lock<std::recursive_mutex>{_sharedObjectsMutex};\
		}\
	protected:\

#define STATIC_OBJECT_CPP(ClassName) \
	ClassName::AutoIncrementID::AutoIncrementID(ClassName::Id id) {\
		static std::atomic<ClassName::Id> lastId = 0;\
		if (id < 0) {\
			_id = ++lastId;\
		} else {\
			lastId = _id = id;\
		}\
	}\
	std::unordered_map<ClassName::Id, ClassName::Ptr> ClassName::_sharedObjects {};\
	std::recursive_mutex ClassName::_sharedObjectsMutex {};\

#define STATIC_OBJECT_EXT(ChildClass, ParentMostClass) \
	public:\
	using Ptr = std::shared_ptr<ChildClass>;\
	using WeakPtr = std::weak_ptr<ChildClass>;\
	ChildClass::Ptr Self() {return std::dynamic_pointer_cast<ChildClass>(ParentMostClass::Self());}\
	template<typename...Args>\
	static ChildClass::Ptr Create(Args&&...args) {\
		return ParentMostClass::Create<ChildClass, Args...>(std::forward<Args>(args)...);\
	}\
	template<typename...Args>\
	static ChildClass::Ptr GetOrCreate(Id id, Args&&...args) {\
		return ParentMostClass::GetOrCreate<ChildClass, Args...>(id, std::forward<Args>(args)...);\
	}\
	friend class ParentMostClass;\
	static void Destroy(ChildClass::Ptr& ptr) {ParentMostClass::Destroy(ptr->GetID());}\
	static ChildClass::Ptr Get(Id id) {return ParentMostClass::Get<ChildClass>(id);}\
	static bool Exists(Id id) {return ParentMostClass::Exists<ChildClass>(id);}\
	operator ParentMostClass::Ptr () {return ParentMostClass::Self();}\
	static void ForEach(std::function<void(ChildClass::Ptr&)>&& func) {\
		ParentMostClass::ForEach([&func](auto& ptr) {\
			if (auto p = std::dynamic_pointer_cast<ChildClass>(ptr); p) {\
				func(p);\
				if (!p) ptr.reset();\
			}\
		});\
	}\
	protected:\

//
