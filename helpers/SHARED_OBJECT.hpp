/**
 * Shared Objects helper v1.0
 * Author: Olivier St-Laurent
 * 
 * This helper will add to a class the ability to loop through all instances of it and keep track of of them, in a thread-safe manner.
 * (Instances are NOT contiguous in memory, hence not cache friendly, so this helper is most useful for code clarity, not performance)
 * 
 * Usage: 
 * 
 * 	// Test.h
 * 		class Test {
 * 			SHARED_OBJECT_H(Test) // Define this class as a shared object here.
 * 			Test() {} // You may define any number of constructors with any number of arguments, with private accessibility
 * 			// Any other private members here
 * 			// You may call Self() from within a member method (except constructor) to get the shared pointer.
 * 		public:
 * 			~Test() { // Destructor must have public accessibility
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
 */

#pragma once

#include <mutex>
#include <vector>
#include <memory>

#define SHARED_OBJECT_H(ClassName) \
	public: using Ptr = std::shared_ptr<ClassName>;\
	public: using WeakPtr = std::weak_ptr<ClassName>;\
	private:\
		static std::vector<WeakPtr> _sharedObjects;\
		static std::recursive_mutex _sharedObjectsMutex;\
		static void Destroy(ClassName*);\
		WeakPtr _self;\
		Ptr Self() {return _self.lock();}\
	public:\
		template<typename...Args>\
		static Ptr Create(Args&&...args) {\
			std::lock_guard lock(_sharedObjectsMutex);\
			Ptr sharedPtr(new ClassName(std::forward<Args>(args)...));\
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
	private:
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
