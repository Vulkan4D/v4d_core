#pragma once
#include <v4d.h>

// Pause current thread for specified duration (ex: 5s = 5 seconds)
#define SLEEP(x) {using namespace std::literals::chrono_literals; std::this_thread::sleep_for(x);}

// Force Inline
#ifdef _MSC_VER
	#define INLINE __forceinline
#else
	#define INLINE __attribute__((always_inline))
#endif


//////////////////////////////////////////////////////////
// Delete Default Constructors

#define DELETE_COPY_CONSTRUCTORS(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete;

#define DELETE_MOVE_CONSTRUCTORS(ClassName) \
	ClassName(ClassName&&) = delete; \
	ClassName& operator=(ClassName&&) = delete;

#define DELETE_COPY_MOVE_CONSTRUCTORS(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete; \
	ClassName(ClassName&&) = delete; \
	ClassName& operator=(ClassName&&) = delete;


//////////////////////////////////////////////////////////
// Library import/export

#define EXTERNC extern "C"
#define EXTERNCPP extern "C++"
#if defined _WINDOWS
	#define DLLEXPORT __declspec(dllexport)
	#define DLLIMPORT __declspec(dllimport)
#else
	#define DLLEXPORT
	#define DLLIMPORT
#endif
#ifdef _V4D_CORE
	#define V4DLIB DLLEXPORT
#else// Project/Module
	#define V4DLIB DLLIMPORT
#endif


//////////////////////////////////////////////////////////
// Signal Handling

#ifdef _LINUX
	#define __ATTACH_OS_SPECIFIC_SIGNALS(handler) \
		signal(SIGHUP, handler); \
		signal(SIGQUIT, handler);
#else
	#define __ATTACH_OS_SPECIFIC_SIGNALS(handler)
#endif
#define ATTACH_SIGNAL_HANDLER(handler) \
	signal(SIGTERM, handler); \
	signal(SIGINT, handler); \
	signal(SIGABRT, handler); \
	signal(SIGFPE, handler); \
	signal(SIGILL, handler); \
	signal(SIGSEGV, handler); \
	__ATTACH_OS_SPECIFIC_SIGNALS(handler)


//////////////////////////////////////////////////////////
// Static Instances Map
#define STATIC_CLASS_INSTANCES(key, className, ...) { \
	static std::mutex staticMu; \
	std::lock_guard staticLock(staticMu); \
	static std::unordered_map<decltype(key), std::shared_ptr<className>> instances {}; \
	if (instances.find(key) == instances.end()) { \
		instances.emplace(key, std::make_shared<className>(__VA_ARGS__)); \
	} \
	std::shared_ptr<className> instance = instances.at(key); \
	return instance; \
}


//////////////////////////////////////////////////////////
// FOREACH macro (maximum of 32 arguments, may increase in the future if needed)

#define __FE_1(__FE, X) __FE(X) 
#define __FE_2(__FE, X, ...) __FE(X)__FE_1(__FE, __VA_ARGS__)
#define __FE_3(__FE, X, ...) __FE(X)__FE_2(__FE, __VA_ARGS__)
#define __FE_4(__FE, X, ...) __FE(X)__FE_3(__FE, __VA_ARGS__)
#define __FE_5(__FE, X, ...) __FE(X)__FE_4(__FE, __VA_ARGS__)
#define __FE_6(__FE, X, ...) __FE(X)__FE_5(__FE, __VA_ARGS__)
#define __FE_7(__FE, X, ...) __FE(X)__FE_6(__FE, __VA_ARGS__)
#define __FE_8(__FE, X, ...) __FE(X)__FE_7(__FE, __VA_ARGS__)
#define __FE_9(__FE, X, ...) __FE(X)__FE_8(__FE, __VA_ARGS__)
#define __FE_10(__FE, X, ...) __FE(X)__FE_9(__FE, __VA_ARGS__)
#define __FE_11(__FE, X, ...) __FE(X)__FE_10(__FE, __VA_ARGS__)
#define __FE_12(__FE, X, ...) __FE(X)__FE_11(__FE, __VA_ARGS__)
#define __FE_13(__FE, X, ...) __FE(X)__FE_12(__FE, __VA_ARGS__)
#define __FE_14(__FE, X, ...) __FE(X)__FE_13(__FE, __VA_ARGS__)
#define __FE_15(__FE, X, ...) __FE(X)__FE_14(__FE, __VA_ARGS__)
#define __FE_16(__FE, X, ...) __FE(X)__FE_15(__FE, __VA_ARGS__)
#define __FE_17(__FE, X, ...) __FE(X)__FE_16(__FE, __VA_ARGS__)
#define __FE_18(__FE, X, ...) __FE(X)__FE_17(__FE, __VA_ARGS__)
#define __FE_19(__FE, X, ...) __FE(X)__FE_18(__FE, __VA_ARGS__)
#define __FE_20(__FE, X, ...) __FE(X)__FE_19(__FE, __VA_ARGS__)
#define __FE_21(__FE, X, ...) __FE(X)__FE_20(__FE, __VA_ARGS__)
#define __FE_22(__FE, X, ...) __FE(X)__FE_21(__FE, __VA_ARGS__)
#define __FE_23(__FE, X, ...) __FE(X)__FE_22(__FE, __VA_ARGS__)
#define __FE_24(__FE, X, ...) __FE(X)__FE_23(__FE, __VA_ARGS__)
#define __FE_25(__FE, X, ...) __FE(X)__FE_24(__FE, __VA_ARGS__)
#define __FE_26(__FE, X, ...) __FE(X)__FE_25(__FE, __VA_ARGS__)
#define __FE_27(__FE, X, ...) __FE(X)__FE_26(__FE, __VA_ARGS__)
#define __FE_28(__FE, X, ...) __FE(X)__FE_27(__FE, __VA_ARGS__)
#define __FE_29(__FE, X, ...) __FE(X)__FE_28(__FE, __VA_ARGS__)
#define __FE_30(__FE, X, ...) __FE(X)__FE_29(__FE, __VA_ARGS__)
#define __FE_31(__FE, X, ...) __FE(X)__FE_30(__FE, __VA_ARGS__)
#define __FE_32(__FE, X, ...) __FE(X)__FE_31(__FE, __VA_ARGS__)
#define __FOR_EACH(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,macroName,...) macroName 
#define FOR_EACH(_MACRO,...) __FOR_EACH(__VA_ARGS__,__FE_32,__FE_31,__FE_30,__FE_29,__FE_28,__FE_27,__FE_26,__FE_25,__FE_24,__FE_23,__FE_22,__FE_21,__FE_20,__FE_19,__FE_18,__FE_17,__FE_16,__FE_15,__FE_14,__FE_13,__FE_12,__FE_11,__FE_10,__FE_9,__FE_8,__FE_7,__FE_6,__FE_5,__FE_4,__FE_3,__FE_2,__FE_1)(_MACRO,__VA_ARGS__)


//////////////////////////////////////////////////////////
// CPU Affinity

#ifdef _WINDOWS
	#define SET_CPU_AFFINITY(n) {\
		DWORD_PTR processAffinityMask = 1 << ((n) % std::thread::hardware_concurrency());\
		if (!SetThreadAffinityMask(GetCurrentThread(), processAffinityMask)) LOG_ERROR("Error calling SetThreadAffinityMask");\
	}
#else
	#define SET_CPU_AFFINITY(n) {\
		cpu_set_t cpuset;\
		CPU_ZERO(&cpuset);\
		CPU_SET((n) % std::thread::hardware_concurrency(), &cpuset);\
		int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);\
		if (rc != 0) LOG_ERROR("Error calling pthread_setaffinity_np: error " << rc)\
		}
#endif

