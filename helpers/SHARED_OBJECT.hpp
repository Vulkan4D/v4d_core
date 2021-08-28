/**
 * Shared Objects helper v1.1
 * Author: Olivier St-Laurent
 * 
 * This helper will add to a class the ability to loop through all instances of it and keep track of them, in a thread-safe manner.
 * (Instances are NOT contiguous in memory, hence not cache friendly, so this helper is most useful for code clarity, not performance)
 * 
 * It consists simply of the macros SHARED_OBJECT_[H|CPP|EXT](), and using them will basically add the following reserved methods:
 * 		* Self() // returns the shared_ptr of this shared object, to be called within another member method
 * 		* Create(...) // Used to instantiate the object, it's the public version of the constructor and takes any argument list compatible with the constructors you define
 * 		* Destroy(this) // To be called in the destructor, specifically with the "this" argument, it effectively removes the object from the global instance list
 * 		* ForEach(func<void|bool(Ptr&)>) // To allow looping through all instances of a shared object using a lambda
 * 		* Sort(func<bool(Ptr&, Ptr&)>) // Sort the global instance list before looping through them, using the usual sorting lambdas (return TRUE if the first argument should be placed BEFORE the second)
 * 
 * Usage:
 * 
 * 	// Test.h
 * 		class Test {
 * 			SHARED_OBJECT_H(Test) // Define this class as a shared object here.
 * 			Test() {} // You may define any number of constructors with any number of arguments, with protected accessibility
 * 			// Any other protected members here
 * 			// You may call Self() from within a member method (except constructor) to get the shared pointer.
 * 		public:
 * 			~Test() { // Destructor must have public accessibility. Also, the destructor must be virtual if we are planning on extending this class.
 * 				Destroy(this); // Must call this first in your destructor
 * 				// Other stuff to do here when destroying the object
 * 			}
 * 			// Any other public members here
 * 		};
 * 
 * 	// Test.cpp
 * 		SHARED_OBJECT_CPP(Test)
 * 
 * 	// main.cpp
 * 		#include "Test.h"
 * 		int main() {
 * 			Test::Ptr myTestObject = Test::Create(); // you may pass any argument list that are compatible with your constructors in Create()
 * 			// myTestObject is a shared pointer to your newly created object
 * 			//...
 * 			Test::ForEach([](Test::Ptr& test){ // Loops through all existing and non-destroyed references to objects of type Test
 * 				// do stuff here with test (it's a shared pointer to your object)
 * 				// optionally return bool here (return false to break the loop)
 * 			});
 * 			//...
 * 			// myTestObject gets destroyed automatically when all references to it are destroyed
 * 		}
 * 
 * 
 * // Inheritance
 * 		// It is also possible to extend a shared object class, while they all share the same global instance list as the parent-most shared object (the class that has the SHARED_OBJECT_H macro defined)
 * 		// To do this, child classes must simply have the macro SHARED_OBJECT_EXT(ChildClass, ParentClass)
 * 		// and define a constructor that invokes a parent constructor
 * 		// The Parent-most shared object must have a virtual destructor, which happens to calls Destroy(this)
 * 
 * 		class Child : public Parent {
 * 			SHARED_OBJECT_EXT(Child, Parent)
 * 			Child() : Parent() {} // Again, you may have any number of arguments that are forwarded from when you call Child::Create()
 * 		  public: // Again, destructor must have public accessibility
 * 			~Child() {
 * 				Destroy(this); // Even though it would have been taken care of in the parent's destructor, it's better that we make sure that the object is gone from the global instance list before we start destroying anything.
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
		static void Destroy(ClassName*);\
		Ptr Self() {return _self.lock();}\
	public:\
		template<class C = ClassName, typename...Args>\
		static Ptr Create(Args&&...args) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			Ptr sharedPtr((ClassName*)new C(std::forward<Args>(args)...));\
			sharedPtr->_self = sharedPtr;\
			_sharedObjects.emplace_back(sharedPtr);\
			return sharedPtr;\
		}\
		static void ForEach(std::function<bool(Ptr&)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			for (auto& t : _sharedObjects) if (auto sharedPtr = t.lock(); sharedPtr) {\
				if (!func(sharedPtr)) break;\
			}\
		}\
		static void ForEach(std::function<void(Ptr&)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			for (auto& t : _sharedObjects) if (auto sharedPtr = t.lock(); sharedPtr) func(sharedPtr);\
		}\
		static void Sort(std::function<bool(Ptr&, Ptr&)>&& func) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			std::sort(_sharedObjects.begin(), _sharedObjects.end(), [&func](WeakPtr& _a, WeakPtr& _b){\
				Ptr a = _a.lock();\
				if (!a) return false;\
				Ptr b = _b.lock();\
				if (!b) return true;\
				return func(a, b);\
			});\
		}\
	protected:
#define SHARED_OBJECT_CPP(ClassName) \
	std::vector<ClassName::WeakPtr> ClassName::_sharedObjects {};\
	std::recursive_mutex ClassName::_sharedObjectsMutex {};\
	void ClassName::Destroy(ClassName* ptr) {\
		std::lock_guard lock(_sharedObjectsMutex);\
		for (auto it = _sharedObjects.begin(); it != _sharedObjects.end(); ++it) if (auto sharedPtr = it->lock(); sharedPtr) {\
			if (sharedPtr.get() == ptr) {\
				it->swap(_sharedObjects.back());\
				_sharedObjects.pop_back();\
				break;\
			}\
		}\
	}
#define SHARED_OBJECT_EXT(ChildClass, ParentClass) \
	public:\
	template<typename...Args>\
	static ParentClass::Ptr Create(Args&&...args) {\
		return ParentClass::Create<ChildClass, Args...>(std::forward<Args>(args)...);\
	}\
	friend class ParentClass;\
	protected:\
	static void Destroy(ChildClass* ptr) {ParentClass::Destroy(ptr);}
