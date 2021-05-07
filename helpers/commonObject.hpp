#pragma once

// Options
	// #define COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET // requires a compiler that supports offsetof() on non-standard-layout types, and may require compiling with the flag -Wno-invalid-offsetof

/*
	CommonObject macro v1.0
	Author: Olivier St-Laurent, May 2021

	Common Objects are practical to automatically be able to loop through all existing objects of a certain type
	
	All Common Objects that are constructed will automatically be added to a global list containing all common objects of that type, and will be removed from that list upon destruction.
	Common objects also implicitly cast to the underlying type's reference or pointer, and can be assigned (=) directly to their underlying type
	Common objects also automatically forward all constructor arguments to the underlying type's constructor, unless you define your own constructor
	
	Usage:
		
	// MyObject.h
		
		// If you don't need any additional members, a one-liner will do:
		COMMON_OBJECT_CLASS (MyObject, UnderlyingObject)
		
		// OR, if you need custom members and/or constructors/destructor, you may wrap it in a class like so:
		class MyObject { // class CANNOT extend from a base class, unless you define COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET
			COMMON_OBJECT (MyObject, UnderlyingObject) // MUST be the first definition of the class, or it's Undefined Behaviour, unless you define COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET
			
			// following members will automatically have public accessibility, you may of course change that if you wish...
			
			int a;
			int b;
			
			MyObject(int a, int b) : obj(), a(a), b(b) { // always initialize obj first (or don't initialize it and it will call its default () constructor)
				// in member functions, use the predefined member 'obj' to access the underlying type's reference or pointer (implicitly cast)
			}
			
		};
		
		// You cannot derive a common object class, unless you define COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET
		
	// MyObject.cpp
		
		COMMON_OBJECT_CPP (MyObject, UnderlyingObject)
		
		// And any other member definitions...
		
	// main.cpp
	
		MyObject obj1;
		MyObject obj2 {arg1,arg2}; // constructor args will be automatically forwarded to the underlying type's constructor, unless you have defined your own

		int main() {
			// Sort the global list before looping through it
			MyObject::Sort([](MyObject* a, MyObject* b){return a->sortIndex < b->sortIndex;});
			
			// Loop through all existing MyObject objects
			MyObject::ForEach([](MyObject* o){
				// use o->obj to access the underlying type
				// or simply dereference o directly as it will automatically cast to the underlying type's reference or pointer as needed
				// you may also use o->someMemberOfUnderlyingType
				// you may return false to stop the loop at any point
			});
			
			// using obj1 directly as an argument to a function can be implicitly cast to the underlying type's reference or pointer
			
			// operator-> is defined so that you can use obj1->someMemberOfUnderlyingType
			
		}
		
*/

#include <mutex>
#include <vector>
#include <functional>
#include <utility>
#include <algorithm>

// You may define this macro with __declspec(dllexport) or __declspec(dllimport) if compiling for Windows
#ifndef IMPORT_OR_EXPORT_COMMON_OBJECT
	#define IMPORT_OR_EXPORT_COMMON_OBJECT
#endif

#ifdef COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET
	#define _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(Object,Member) offsetof(Object,Member)
#else
	#define _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(Object,Member) 0
#endif

#define COMMON_OBJECT(ObjClass, UnderlyingClass) \
	protected:\
	class IMPORT_OR_EXPORT_COMMON_OBJECT UnderlyingCommonObjectContainer {\
		friend class ObjClass;\
		UnderlyingClass obj;\
		static std::mutex mu;\
		static std::vector<ObjClass*> objs;\
		template<typename...Args>\
		UnderlyingCommonObjectContainer(Args&&...args) : obj(std::forward<Args>(args)...) {\
			constexpr int64_t _ptrOffset = _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(ObjClass, obj);\
			std::lock_guard lock(mu);\
			if constexpr (_ptrOffset == 0)\
				objs.emplace_back(reinterpret_cast<ObjClass*>(this));\
			else\
				objs.emplace_back(reinterpret_cast<ObjClass*>( (char*)(this) - _ptrOffset ));\
		}\
		~UnderlyingCommonObjectContainer() {\
			constexpr int64_t _ptrOffset = _COMMON_OBJECT_FLEXIBLE_PTR_OFFSET_OF_(ObjClass, obj);\
			std::lock_guard lock(mu);\
			if constexpr (_ptrOffset == 0)\
				std::erase(objs, reinterpret_cast<ObjClass*>(this));\
			else\
				std::erase(objs, reinterpret_cast<ObjClass*>( (char*)(this) - _ptrOffset ));\
		}\
		public:\
		operator UnderlyingClass&() {return obj;}\
		operator UnderlyingClass*() {return &obj;}\
	} obj;\
	public:\
	template<typename...Args>\
	ObjClass(Args&&...args) : obj(std::forward<Args>(args)...) {}\
	template<typename T>\
	UnderlyingClass& operator=(const T& other) {return obj.obj = other;}\
	template<typename T>\
	UnderlyingClass& operator=(T&& other) {return obj.obj = std::move(other);}\
	UnderlyingClass* operator->() {return &obj.obj;}\
	operator UnderlyingClass&() {return obj.obj;}\
	operator UnderlyingClass*() {return &obj.obj;}\
	static void Sort(std::function<bool(ObjClass*,ObjClass*)>&& f) {\
		std::lock_guard lock(UnderlyingCommonObjectContainer::mu);\
		std::sort(\
			UnderlyingCommonObjectContainer::objs.begin(),\
			UnderlyingCommonObjectContainer::objs.end(),\
			std::forward<std::function<bool(ObjClass*,ObjClass*)>>(f)\
		);\
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
#define COMMON_OBJECT_CLASS(ObjClass, UnderlyingClass) \
	class ObjClass { COMMON_OBJECT(ObjClass, UnderlyingClass) };
#define COMMON_OBJECT_CPP(ObjClass, UnderlyingClass) \
	std::mutex ObjClass::UnderlyingCommonObjectContainer::mu {};\
	std::vector<ObjClass*> ObjClass::UnderlyingCommonObjectContainer::objs {};
