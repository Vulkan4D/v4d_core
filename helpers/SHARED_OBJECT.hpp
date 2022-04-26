/**
 * Shared Objects helper v2.0 - 2022-04-25
 * Author: Olivier St-Laurent
 * 
 * This helper will add to a class the ability to loop through all instances of it and keep track of them, in a thread-safe manner.
 * (Instances are NOT contiguous in memory, hence not cache friendly, so this helper is most useful for code clarity, not performance)
 * 
 * It consists simply of the macros SHARED_OBJECT_[H|CPP|EXT](), and using them will basically add the following reserved methods:
 * 		* Self() // returns the shared_ptr of this shared object, to be called within another member method
 * 		* Make(...) // Used to instantiate the object, it's the public version of the constructor and takes any argument list compatible with the constructors you define
 * 		* ForEach(func<void(Ptr&)>) // To allow looping through all instances of a shared object using a lambda
 * 		* ForEachOrBreak(func<bool(Ptr&)>) // To allow looping through all instances of a shared object using a lambda, only loops through valid object pointers
 * 		* ForEachWeak(func<bool(size_t index, Ptr)>) // To allow looping through all instances of a shared object using a lambda, also includes the index, and ensures the lambda is called for each and every object, hence we MUST check if the ptr is valid before using it
 * 		* Sort(func<bool(Ptr&, Ptr&)>) // Sort the global instance list before looping through them, using the usual sorting lambdas (return TRUE if the first argument should be placed BEFORE the second) and also collects the garbage
 * 		* Count() // Returns the current number of instances of this type
 * 		* GetLock() // static method that returns a lock on all instances of this type for Add/Remove/ForEach operations (but not Make)
 * 		* CollectGarbage() // Removes all invalid (expired) pointers from the global instance list (this is also done during Sort)
 * Usage:
 * 
 * 	// Test.h
 * 		class Test {
 * 			SHARED_OBJECT_H(Test) // Define this class as a shared object here.
 * 			Test() {} // You may define any number of constructors with any number of arguments, with private/protected accessibility
 * 			// Any other private/protected members here
 * 			// You may call Self() from within a member method (except the constructors/destructor) to get the shared pointer.
 * 		public:
 * 			virtual ~Test() {} // Destructor must have public accessibility, if defined. Also, the destructor must be virtual if we are planning on extending this class.
 * 			// Any other public members here
 * 		};
 * 
 * 	// Test.cpp
 * 		SHARED_OBJECT_CPP(Test)
 * 
 * 	// main.cpp
 * 		#include "Test.h"
 * 		int main() {
 * 			Test::Ptr myTestObject = Test::Make(); // you may pass any argument list that are compatible with your constructors in Make()
 * 			// myTestObject is a shared pointer to your newly created object
 * 			//...
 * 			Test::ForEach([](Test::Ptr& test){ // Loops through all existing and non-destroyed references to objects of type Test
 * 				// do stuff here with test (it's a shared pointer to your object)
 * 			});
 * 			Test::ForEachOrBreak([](Test::Ptr& test) -> bool { // Loops through all existing and non-destroyed references to objects of type Test, with the possibility to break the loop by returning false
 * 				// do stuff here with test (it's a shared pointer to your object)
 * 				// return bool here (return true to continue, false to break the loop)
 * 			});
 * 			Test::ForEachWeak([](size_t index, Test::Ptr test){ // Loops through all references to objects of type Test, even if it's invalid (expired)
 * 				if (test) { // We must make sure the pointer is not expired
 * 					// do stuff here with test (it's a shared pointer to your object) or with it's index which will NOT change until a call to Sort() or CollectGarbage()
 * 				}
 * 			});
 * 			//...
 * 			// myTestObject gets destroyed automatically when all references to it are destroyed
 * 		}
 * 
 * 
 * // Inheritance
 * 		// It is also possible to extend a shared object class, while they all share the same global instance list as the parent-most shared object (the class that has the SHARED_OBJECT_H macro defined)
 * 		// To do this, child classes must simply have the macro SHARED_OBJECT_EXT(ChildClass, ParentMostClass)
 * 		// and define a constructor that invokes a parent constructor
 * 		// The Parent-most shared object must have a virtual destructor
 * 
 * 		class Child : public Parent {
 * 			SHARED_OBJECT_EXT(Child, Parent)
 * 			Child() : Parent() {} // Again, you may have any number of arguments that are forwarded from when you call Child::Make()
 * 		  public: // Again, destructor must have public accessibility
 * 			~Child() {
 * 				// delete stuff...
 * 			}
 * 		};
 * 
 */

#pragma once

#include <mutex>
#include <vector>
#include <memory>

#define SHARED_OBJECT_H(ClassName) \
	public:\
		using Ptr = std::shared_ptr<ClassName>;\
		using WeakPtr = std::weak_ptr<ClassName>;\
	private:\
		static std::vector<WeakPtr> _sharedObjects;\
		static std::recursive_mutex _sharedObjectsMutex;\
		WeakPtr _self;\
	protected:\
		Ptr Self() {return _self.lock();}\
	public:\
		template<class C = ClassName, typename...Args>\
		static auto Make(Args&&...args) {\
			typename C::Ptr sharedPtr(new C(std::forward<Args>(args)...));\
			sharedPtr->_self = sharedPtr;\
			std::lock_guard lock(_sharedObjectsMutex);\
			auto availableSlot = std::find_if(_sharedObjects.begin(), _sharedObjects.end(), [](WeakPtr& o){return o.expired();});\
			if (availableSlot == _sharedObjects.end()) _sharedObjects.emplace_back(sharedPtr);\
			else *availableSlot = sharedPtr;\
			return sharedPtr;\
		}\
		static void ForEach(std::function<void(Ptr&)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			for (auto& t : _sharedObjects) if (auto sharedPtr = t.lock(); sharedPtr) func(sharedPtr);\
		}\
		static void ForEachOrBreak(std::function<bool(Ptr&)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			for (auto& t : _sharedObjects) if (auto sharedPtr = t.lock(); sharedPtr) {\
				if (!func(sharedPtr)) break;\
			}\
		}\
		static void ForEachWeak(std::function<void(size_t index, Ptr)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			for (size_t i = 0; i < _sharedObjects.size(); ++i) func(i, _sharedObjects[i].lock());\
		}\
		static size_t Count() {\
			std::lock_guard lock(_sharedObjectsMutex);\
			return _sharedObjects.size();\
		}\
		static void Sort(std::function<bool(Ptr&, Ptr&)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			std::sort(_sharedObjects.begin(), _sharedObjects.end(), [&func](WeakPtr& _a, WeakPtr& _b){\
				Ptr b = _b.lock();\
				if (!b) return true;\
				Ptr a = _a.lock();\
				if (!a) return false;\
				return func(a, b);\
			});\
			/*After a sort, All expired pointers will be at the end, hence it is safe to erase from the first expired all the way until the end*/\
			_sharedObjects.erase(std::find_if(_sharedObjects.begin(), _sharedObjects.end(), [](WeakPtr& o){return o.expired();}), _sharedObjects.end());\
		}\
		/*NOTE: Sort() will also collect the garbage*/\
		static void CollectGarbage() {\
			std::lock_guard lock(_sharedObjectsMutex);\
			_sharedObjects.erase(std::remove_if(_sharedObjects.begin(), _sharedObjects.end(), [](WeakPtr& o){return o.expired();}), _sharedObjects.end());\
		}\
		static std::unique_lock<std::recursive_mutex> GetLock() {\
			return std::unique_lock<std::recursive_mutex>{_sharedObjectsMutex};\
		}\
	protected:\

#define SHARED_OBJECT_CPP(ClassName) \
	std::vector<ClassName::WeakPtr> ClassName::_sharedObjects {};\
	std::recursive_mutex ClassName::_sharedObjectsMutex {};\

#define SHARED_OBJECT_EXT(ChildClass, ParentMostClass) \
	public:\
	using Ptr = std::shared_ptr<ChildClass>;\
	using WeakPtr = std::weak_ptr<ChildClass>;\
	template<typename...Args>\
	static ChildClass::Ptr Make(Args&&...args) {\
		return ParentMostClass::Make<ChildClass, Args...>(std::forward<Args>(args)...);\
	}\
	friend class ParentMostClass;\
	protected:\

//
