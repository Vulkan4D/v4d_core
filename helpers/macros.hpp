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


// Assert
#ifdef _DEBUG
	#define DEBUG_ASSERT(expression) assert(expression);
	#define DEBUG_ASSERT_WARN(expression, msg) {\
		if (!(expression)) {\
			LOG_WARN(msg)\
		}\
	}
	#define DEBUG_ASSERT_ERROR(expression, msg) {\
		if (!(expression)) {\
			LOG_ERROR(msg)\
		}\
	}
#else
	#define DEBUG_ASSERT(expression)
	#define DEBUG_ASSERT_WARN(expression, msg)
	#define DEBUG_ASSERT_ERROR(expression, msg)
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
	DELETE_COPY_CONSTRUCTORS(ClassName)\
	DELETE_MOVE_CONSTRUCTORS(ClassName)


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
// FOREACH macro (maximum of 64 arguments, may increase in the future if needed)

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
#define __FE_33(__FE, X, ...) __FE(X)__FE_32(__FE, __VA_ARGS__)
#define __FE_34(__FE, X, ...) __FE(X)__FE_33(__FE, __VA_ARGS__)
#define __FE_35(__FE, X, ...) __FE(X)__FE_34(__FE, __VA_ARGS__)
#define __FE_36(__FE, X, ...) __FE(X)__FE_35(__FE, __VA_ARGS__)
#define __FE_37(__FE, X, ...) __FE(X)__FE_36(__FE, __VA_ARGS__)
#define __FE_38(__FE, X, ...) __FE(X)__FE_37(__FE, __VA_ARGS__)
#define __FE_39(__FE, X, ...) __FE(X)__FE_38(__FE, __VA_ARGS__)
#define __FE_40(__FE, X, ...) __FE(X)__FE_39(__FE, __VA_ARGS__)
#define __FE_41(__FE, X, ...) __FE(X)__FE_40(__FE, __VA_ARGS__)
#define __FE_42(__FE, X, ...) __FE(X)__FE_41(__FE, __VA_ARGS__)
#define __FE_43(__FE, X, ...) __FE(X)__FE_42(__FE, __VA_ARGS__)
#define __FE_44(__FE, X, ...) __FE(X)__FE_43(__FE, __VA_ARGS__)
#define __FE_45(__FE, X, ...) __FE(X)__FE_44(__FE, __VA_ARGS__)
#define __FE_46(__FE, X, ...) __FE(X)__FE_45(__FE, __VA_ARGS__)
#define __FE_47(__FE, X, ...) __FE(X)__FE_46(__FE, __VA_ARGS__)
#define __FE_48(__FE, X, ...) __FE(X)__FE_47(__FE, __VA_ARGS__)
#define __FE_49(__FE, X, ...) __FE(X)__FE_48(__FE, __VA_ARGS__)
#define __FE_50(__FE, X, ...) __FE(X)__FE_49(__FE, __VA_ARGS__)
#define __FE_51(__FE, X, ...) __FE(X)__FE_50(__FE, __VA_ARGS__)
#define __FE_52(__FE, X, ...) __FE(X)__FE_51(__FE, __VA_ARGS__)
#define __FE_53(__FE, X, ...) __FE(X)__FE_52(__FE, __VA_ARGS__)
#define __FE_54(__FE, X, ...) __FE(X)__FE_53(__FE, __VA_ARGS__)
#define __FE_55(__FE, X, ...) __FE(X)__FE_54(__FE, __VA_ARGS__)
#define __FE_56(__FE, X, ...) __FE(X)__FE_55(__FE, __VA_ARGS__)
#define __FE_57(__FE, X, ...) __FE(X)__FE_56(__FE, __VA_ARGS__)
#define __FE_58(__FE, X, ...) __FE(X)__FE_57(__FE, __VA_ARGS__)
#define __FE_59(__FE, X, ...) __FE(X)__FE_58(__FE, __VA_ARGS__)
#define __FE_60(__FE, X, ...) __FE(X)__FE_59(__FE, __VA_ARGS__)
#define __FE_61(__FE, X, ...) __FE(X)__FE_60(__FE, __VA_ARGS__)
#define __FE_62(__FE, X, ...) __FE(X)__FE_61(__FE, __VA_ARGS__)
#define __FE_63(__FE, X, ...) __FE(X)__FE_62(__FE, __VA_ARGS__)
#define __FE_64(__FE, X, ...) __FE(X)__FE_63(__FE, __VA_ARGS__)
#define __FOR_EACH(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,macroName,...) macroName 
#define FOR_EACH(_MACRO,...) __FOR_EACH(__VA_ARGS__,__FE_64,__FE_63,__FE_62,__FE_61,__FE_60,__FE_59,__FE_58,__FE_57,__FE_56,__FE_55,__FE_54,__FE_53,__FE_52,__FE_51,__FE_50,__FE_49,__FE_48,__FE_47,__FE_46,__FE_45,__FE_44,__FE_43,__FE_42,__FE_41,__FE_40,__FE_39,__FE_38,__FE_37,__FE_36,__FE_35,__FE_34,__FE_33,__FE_32,__FE_31,__FE_30,__FE_29,__FE_28,__FE_27,__FE_26,__FE_25,__FE_24,__FE_23,__FE_22,__FE_21,__FE_20,__FE_19,__FE_18,__FE_17,__FE_16,__FE_15,__FE_14,__FE_13,__FE_12,__FE_11,__FE_10,__FE_9,__FE_8,__FE_7,__FE_6,__FE_5,__FE_4,__FE_3,__FE_2,__FE_1)(_MACRO,__VA_ARGS__)


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

