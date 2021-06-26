/*

CommonObject macro v1.1
Author: Olivier St-Laurent, May 2021 - June 2021

Common Objects are practical to be able to loop through all existing objects of a certain type

All Common Objects that are constructed at ay point during execution, will automatically be added to a global list containing all common objects of that type, and will be removed from that list upon destruction.
Constructing, Destructing, Sorting and Looping are synchronized operations and thread safe, locked via a global mutex for that type.
Common objects also implicitly cast to the underlying type's reference or pointer, and can be assigned (=) directly to their underlying type.
Common objects also automatically forward all constructor arguments to the underlying type's constructor, unless you define your own constructor, in which case you may construct obj() for the underlying type.
The underlying types are stored inline on the stack and a Common Object will not take more memory than the underlying type itself, unless you define your own wrapper class of course.
The global object lists are stored statically and they store pointers to the underlying types, so the raw underlying objects are not necessarily all contiguous in memory.

Usage:


// Options
	#define COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET // requires a compiler that supports offsetof() on non-standard-layout types, and may require compiling with -Wno-invalid-offsetof


// MyObject.h
	
	// If you don't need any additional members, a one-liner will do:
	COMMON_OBJECT_CLASS (MyObject, UnderlyingObject)
	
	// OR, if you need custom members and/or constructors/destructor, you may wrap it in a class like so:
	class MyObject {
		COMMON_OBJECT (MyObject, UnderlyingObject) // MUST be the first definition of the class, unless you define the option COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET
		
		// If MyObject class is moveable/copyable:
		COMMON_OBJECT_MOVEABLE(MyObject)
		COMMON_OBJECT_COPYABLE(MyObject)
		// You may also want to implement your own a Copy/Move constructor and assignment operators instead, just don't forget to copy/move the 'obj' member (refer to the macro's body for an example)
		
		// OR if MyObject class is NOT moveable/copyable:
		COMMON_OBJECT_NOT_MOVEABLE(MyObject)
		COMMON_OBJECT_NOT_COPYABLE(MyObject)
		
		// following members will automatically have public accessibility, you may of course change that if you wish...
		
		int a;
		int b;
		
		MyObject(int a, int b) : obj(), a(a), b(b) { // always initialize obj first (or don't initialize it and it will call its default () constructor)
			// in member functions, use the predefined member 'obj' to access the underlying type's reference or pointer (implicitly cast)
		}
		
		~MyObject() {
			//...
		}
		
		// You may NOT overload operator-> because this implementation already overloaded it to access the underlying type
	};
	
	// You cannot extend or derive a common object class (nor have virtual methods) unless you define the option COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET
	// however, the underlying object may be a derived class.
	
	
// MyObject.cpp
	
	COMMON_OBJECT_CPP (MyObject, UnderlyingObject)
	
	// And any other member definitions...


// main.cpp

	MyObject obj1;
	MyObject obj2 {arg1,arg2}; // constructor args will be automatically forwarded to the underlying type's constructor, unless you have defined your own

	int main() {
		
		// using obj1 directly as an argument to a function can be implicitly cast to the underlying type's reference or pointer
		
		// operator-> is overloaded, so that you can use obj1->someMemberOfUnderlyingType
		
		// You may sort the global list before looping through it, with your own sort lambda
		MyObject::Sort([](MyObject* a, MyObject* b){return a->sortValue < b->sortValue;});
		
		// You may loop through all existing MyObject objects using your own lambda
		MyObject::ForEach([](MyObject* myObjectPtr){
			// use myObjectPtr->obj to access the underlying type
			// or simply dereference myObjectPtr directly as it will automatically cast to the underlying type's reference or pointer as needed
			// if it is dereferenced, you may also use myObject->someMemberOfUnderlyingType
			// you may return false to break the loop at any point (or return true to continue to the next object)
		});
		
		// You may want to swap the underlying object:
		obj1.Swap(obj2); // This ONLY swaps the underlying object, not any additional data that could have been defined in the wrapper class.
		
	}


// Special usage:

	// Compiling a shared library (DLL) on windows to store the global object list:
	// you may optionally add a 3rd argument to the macros COMMON_OBJECT_CLASS or COMMON_OBJECT with either __declspec(dllexport) or __declspec(dllimport)
	// depending on whether you are compiling the shared library or the executable, respectively.


*/

#pragma once

#include <mutex>
#include <vector>
#include <functional>
#include <utility>
#include <algorithm>
#include <concepts>
#include <type_traits>

#ifndef _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_
	#ifdef COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET
		#include <cstddef>
		#define _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(Object,Member) offsetof(Object,Member)
	#else
		#define _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(Object,Member) 0
	#endif
#endif

#define COMMON_OBJECT(ObjClass, UnderlyingClass, .../* may optionally have a 3rd argument with either __declspec(dllexport) or __declspec(dllimport) */) \
	public:\
	class __VA_ARGS__ UnderlyingCommonObjectContainer {\
		friend class ObjClass;\
		UnderlyingClass obj;\
		static std::mutex mu;\
		static std::vector<ObjClass*> objs;\
		template<typename...Args> requires std::is_constructible_v<UnderlyingClass, Args...>\
		UnderlyingCommonObjectContainer(Args&&...args) : obj(std::forward<Args>(args)...) {\
			Insert();\
		}\
		void Insert() {\
			ObjClass* ptr;\
			constexpr int64_t _ptrOffset = _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(ObjClass, obj);\
			if constexpr (_ptrOffset == 0)\
				ptr = reinterpret_cast<ObjClass*>(this);\
			else\
				ptr = reinterpret_cast<ObjClass*>( reinterpret_cast<char*>(this) - _ptrOffset );\
			std::lock_guard lock(mu);\
			if (std::count(objs.begin(), objs.end(), ptr) == 0) objs.emplace_back(ptr);\
		}\
		void Erase() {\
			ObjClass* ptr;\
			constexpr int64_t _ptrOffset = _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(ObjClass, obj);\
			if constexpr (_ptrOffset == 0)\
				ptr = reinterpret_cast<ObjClass*>(this);\
			else\
				ptr = reinterpret_cast<ObjClass*>( reinterpret_cast<char*>(this) - _ptrOffset );\
			std::lock_guard lock(mu);\
			std::erase(objs, ptr);\
		}\
		~UnderlyingCommonObjectContainer() {Erase();}\
		public:\
		UnderlyingClass& operator=(const UnderlyingClass& other) {return obj = other;}\
		UnderlyingClass* operator->() {return &obj;}\
		operator const UnderlyingClass&() const {return obj;}\
		operator UnderlyingClass&() {return obj;}\
		operator const UnderlyingClass* const() const {return &obj;}\
		operator UnderlyingClass*() {return &obj;}\
		UnderlyingCommonObjectContainer(UnderlyingCommonObjectContainer&& other) {*this = std::move(other);}\
		UnderlyingCommonObjectContainer& operator= (UnderlyingCommonObjectContainer&& other) {\
			if (this != &other) {\
				obj = std::move(other.obj);\
				other.Erase();\
				Insert();\
			}\
			return *this;\
		}\
		UnderlyingCommonObjectContainer(const UnderlyingCommonObjectContainer& other) {*this = other;}\
		UnderlyingCommonObjectContainer& operator= (const UnderlyingCommonObjectContainer& other) {\
			if (this != &other) {\
				obj = other.obj;\
				Insert();\
			}\
			return *this;\
		}\
	} obj;\
	template<typename...Args> requires std::is_constructible_v<UnderlyingClass, Args...>\
	ObjClass(Args&&...args) : obj(std::forward<Args>(args)...) {}\
	ObjClass& operator=(const UnderlyingClass& other) {obj = other; return *this;}\
	ObjClass& operator=(UnderlyingClass&& other) {obj = std::move(other); return *this;}\
	UnderlyingClass* operator->() {return &obj.obj;}\
	operator UnderlyingClass&() {return obj.obj;}\
	operator UnderlyingClass*() {return &obj.obj;}\
	template<typename T>\
	void Swap(T& other) requires std::is_same_v<T, ObjClass> && std::is_nothrow_swappable_v<UnderlyingClass> {\
		std::lock_guard lock(UnderlyingCommonObjectContainer::mu);\
		std::swap(obj.obj, other.obj.obj);\
	}\
	static void Sort(std::function<bool(ObjClass*,ObjClass*)>&& f) {\
		std::lock_guard lock(UnderlyingCommonObjectContainer::mu);\
		std::sort(\
			UnderlyingCommonObjectContainer::objs.begin(),\
			UnderlyingCommonObjectContainer::objs.end(),\
			std::forward<std::function<bool(ObjClass*,ObjClass*)>>(f)\
		);\
	}\
	static size_t Count() {\
		std::lock_guard lock(UnderlyingCommonObjectContainer::mu);\
		return UnderlyingCommonObjectContainer::objs.size();\
	}\
	static void ForEach(std::function<void(ObjClass*)>&& f) {\
		std::lock_guard lock(UnderlyingCommonObjectContainer::mu);\
		for (auto* o : UnderlyingCommonObjectContainer::objs) f(o);\
	}\
	static void ForEach(std::function<bool(ObjClass*)>&& f) {\
		std::lock_guard lock(UnderlyingCommonObjectContainer::mu);\
		for (auto* o : UnderlyingCommonObjectContainer::objs) {\
			if(!f(o)) break;\
		}\
	}

#define COMMON_OBJECT_MOVEABLE(ObjClass)\
	ObjClass(ObjClass&& other) = default;\
	ObjClass& operator= (ObjClass&& other) = default;

#define COMMON_OBJECT_COPYABLE(ObjClass)\
	ObjClass(const ObjClass& other) = default;\
	ObjClass& operator= (const ObjClass& other) = default;
	
#define COMMON_OBJECT_NOT_MOVEABLE(ObjClass)\
	ObjClass(ObjClass&& other) = delete;\
	ObjClass& operator= (ObjClass&& other) = delete;

#define COMMON_OBJECT_NOT_COPYABLE(ObjClass)\
	ObjClass(const ObjClass& other) = delete;\
	ObjClass& operator= (const ObjClass& other) = delete;
	
#define COMMON_OBJECT_CLASS(ObjClass, UnderlyingClass, .../* may optionally have a 3rd argument with either __declspec(dllexport) or __declspec(dllimport) */) \
	class ObjClass {\
		COMMON_OBJECT(ObjClass, UnderlyingClass, __VA_ARGS__)\
		COMMON_OBJECT_MOVEABLE(ObjClass)\
		COMMON_OBJECT_COPYABLE(ObjClass)\
	};

#define COMMON_OBJECT_CPP(ObjClass, UnderlyingClass) \
	std::mutex ObjClass::UnderlyingCommonObjectContainer::mu {};\
	std::vector<ObjClass*> ObjClass::UnderlyingCommonObjectContainer::objs {};

