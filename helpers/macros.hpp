#pragma once

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
	#define ASSERT_OR_RETURN_FALSE(expression) if (!(expression)) {LOG_ERROR("ASSERTION FAILED: " #expression) return false;}
#else
	#define DEBUG_ASSERT(expression)
	#define DEBUG_ASSERT_WARN(expression, msg)
	#define DEBUG_ASSERT_ERROR(expression, msg)
	#define ASSERT_OR_RETURN_FALSE(expression) if (!(expression)) {return false;}
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
// Signal Handling

#ifdef _LINUX
	#define __V4D__ATTACH_OS_SPECIFIC_SIGNALS(handler) \
		signal(SIGHUP, handler); \
		signal(SIGQUIT, handler);
#else
	#define __V4D__ATTACH_OS_SPECIFIC_SIGNALS(handler)
#endif
#define ATTACH_SIGNAL_HANDLER(handler) \
	signal(SIGTERM, handler); \
	signal(SIGINT, handler); \
	signal(SIGABRT, handler); \
	signal(SIGFPE, handler); \
	signal(SIGILL, handler); \
	signal(SIGSEGV, handler); \
	__V4D__ATTACH_OS_SPECIFIC_SIGNALS(handler)


//////////////////////////////////////////////////////////
// Static Instances Map
#define STATIC_CLASS_INSTANCES_CPP(key, ClassName, ...) { \
	static std::mutex staticMu; \
	std::lock_guard staticLock(staticMu); \
	static std::unordered_map<std::decay<decltype(key)>::type, std::weak_ptr<ClassName>> instances {}; \
	auto ptr = instances[key].lock(); \
	if (!ptr) { \
		instances[key] = ptr = std::make_shared<ClassName>(__VA_ARGS__); \
	} \
	return ptr; \
}
#define COMMON_INSTANCES_H(KeyType, ClassName) \
	static std::mutex staticCommonInstancesMutex; \
	static std::unordered_map<KeyType, std::weak_ptr<ClassName>> commonInstances; \
	public: \
	static void ForEach(std::function<void(std::shared_ptr<ClassName>&)>&&func) { \
		std::lock_guard staticLock(staticCommonInstancesMutex); \
		for (auto&[key, weakInstance] : commonInstances) { \
			auto instance = weakInstance.lock(); \
			if (instance) func(instance); \
		} \
	}
#define COMMON_INSTANCES_CPP(KeyType, key, ClassName, ...) { \
	std::lock_guard staticLock(staticCommonInstancesMutex); \
	auto ptr = commonInstances[key].lock(); \
	if (!ptr) { \
		commonInstances[key] = ptr = std::make_shared<ClassName>(__VA_ARGS__); \
	} \
	return ptr; \
} \
std::mutex ClassName::staticCommonInstancesMutex {}; \
std::unordered_map<KeyType, std::weak_ptr<ClassName>> ClassName::commonInstances {};


//////////////////////////////////////////////////////////
// FOREACH macro (maximum of 256 arguments, may increase in the future if needed)

#define __V4D__FE_1(__FE, X) __FE(X) 
#define __V4D__FE_2(__FE, X, ...) __FE(X)__V4D__FE_1(__FE, __VA_ARGS__)
#define __V4D__FE_3(__FE, X, ...) __FE(X)__V4D__FE_2(__FE, __VA_ARGS__)
#define __V4D__FE_4(__FE, X, ...) __FE(X)__V4D__FE_3(__FE, __VA_ARGS__)
#define __V4D__FE_5(__FE, X, ...) __FE(X)__V4D__FE_4(__FE, __VA_ARGS__)
#define __V4D__FE_6(__FE, X, ...) __FE(X)__V4D__FE_5(__FE, __VA_ARGS__)
#define __V4D__FE_7(__FE, X, ...) __FE(X)__V4D__FE_6(__FE, __VA_ARGS__)
#define __V4D__FE_8(__FE, X, ...) __FE(X)__V4D__FE_7(__FE, __VA_ARGS__)
#define __V4D__FE_9(__FE, X, ...) __FE(X)__V4D__FE_8(__FE, __VA_ARGS__)
#define __V4D__FE_10(__FE, X, ...) __FE(X)__V4D__FE_9(__FE, __VA_ARGS__)
#define __V4D__FE_11(__FE, X, ...) __FE(X)__V4D__FE_10(__FE, __VA_ARGS__)
#define __V4D__FE_12(__FE, X, ...) __FE(X)__V4D__FE_11(__FE, __VA_ARGS__)
#define __V4D__FE_13(__FE, X, ...) __FE(X)__V4D__FE_12(__FE, __VA_ARGS__)
#define __V4D__FE_14(__FE, X, ...) __FE(X)__V4D__FE_13(__FE, __VA_ARGS__)
#define __V4D__FE_15(__FE, X, ...) __FE(X)__V4D__FE_14(__FE, __VA_ARGS__)
#define __V4D__FE_16(__FE, X, ...) __FE(X)__V4D__FE_15(__FE, __VA_ARGS__)
#define __V4D__FE_17(__FE, X, ...) __FE(X)__V4D__FE_16(__FE, __VA_ARGS__)
#define __V4D__FE_18(__FE, X, ...) __FE(X)__V4D__FE_17(__FE, __VA_ARGS__)
#define __V4D__FE_19(__FE, X, ...) __FE(X)__V4D__FE_18(__FE, __VA_ARGS__)
#define __V4D__FE_20(__FE, X, ...) __FE(X)__V4D__FE_19(__FE, __VA_ARGS__)
#define __V4D__FE_21(__FE, X, ...) __FE(X)__V4D__FE_20(__FE, __VA_ARGS__)
#define __V4D__FE_22(__FE, X, ...) __FE(X)__V4D__FE_21(__FE, __VA_ARGS__)
#define __V4D__FE_23(__FE, X, ...) __FE(X)__V4D__FE_22(__FE, __VA_ARGS__)
#define __V4D__FE_24(__FE, X, ...) __FE(X)__V4D__FE_23(__FE, __VA_ARGS__)
#define __V4D__FE_25(__FE, X, ...) __FE(X)__V4D__FE_24(__FE, __VA_ARGS__)
#define __V4D__FE_26(__FE, X, ...) __FE(X)__V4D__FE_25(__FE, __VA_ARGS__)
#define __V4D__FE_27(__FE, X, ...) __FE(X)__V4D__FE_26(__FE, __VA_ARGS__)
#define __V4D__FE_28(__FE, X, ...) __FE(X)__V4D__FE_27(__FE, __VA_ARGS__)
#define __V4D__FE_29(__FE, X, ...) __FE(X)__V4D__FE_28(__FE, __VA_ARGS__)
#define __V4D__FE_30(__FE, X, ...) __FE(X)__V4D__FE_29(__FE, __VA_ARGS__)
#define __V4D__FE_31(__FE, X, ...) __FE(X)__V4D__FE_30(__FE, __VA_ARGS__)
#define __V4D__FE_32(__FE, X, ...) __FE(X)__V4D__FE_31(__FE, __VA_ARGS__)
#define __V4D__FE_33(__FE, X, ...) __FE(X)__V4D__FE_32(__FE, __VA_ARGS__)
#define __V4D__FE_34(__FE, X, ...) __FE(X)__V4D__FE_33(__FE, __VA_ARGS__)
#define __V4D__FE_35(__FE, X, ...) __FE(X)__V4D__FE_34(__FE, __VA_ARGS__)
#define __V4D__FE_36(__FE, X, ...) __FE(X)__V4D__FE_35(__FE, __VA_ARGS__)
#define __V4D__FE_37(__FE, X, ...) __FE(X)__V4D__FE_36(__FE, __VA_ARGS__)
#define __V4D__FE_38(__FE, X, ...) __FE(X)__V4D__FE_37(__FE, __VA_ARGS__)
#define __V4D__FE_39(__FE, X, ...) __FE(X)__V4D__FE_38(__FE, __VA_ARGS__)
#define __V4D__FE_40(__FE, X, ...) __FE(X)__V4D__FE_39(__FE, __VA_ARGS__)
#define __V4D__FE_41(__FE, X, ...) __FE(X)__V4D__FE_40(__FE, __VA_ARGS__)
#define __V4D__FE_42(__FE, X, ...) __FE(X)__V4D__FE_41(__FE, __VA_ARGS__)
#define __V4D__FE_43(__FE, X, ...) __FE(X)__V4D__FE_42(__FE, __VA_ARGS__)
#define __V4D__FE_44(__FE, X, ...) __FE(X)__V4D__FE_43(__FE, __VA_ARGS__)
#define __V4D__FE_45(__FE, X, ...) __FE(X)__V4D__FE_44(__FE, __VA_ARGS__)
#define __V4D__FE_46(__FE, X, ...) __FE(X)__V4D__FE_45(__FE, __VA_ARGS__)
#define __V4D__FE_47(__FE, X, ...) __FE(X)__V4D__FE_46(__FE, __VA_ARGS__)
#define __V4D__FE_48(__FE, X, ...) __FE(X)__V4D__FE_47(__FE, __VA_ARGS__)
#define __V4D__FE_49(__FE, X, ...) __FE(X)__V4D__FE_48(__FE, __VA_ARGS__)
#define __V4D__FE_50(__FE, X, ...) __FE(X)__V4D__FE_49(__FE, __VA_ARGS__)
#define __V4D__FE_51(__FE, X, ...) __FE(X)__V4D__FE_50(__FE, __VA_ARGS__)
#define __V4D__FE_52(__FE, X, ...) __FE(X)__V4D__FE_51(__FE, __VA_ARGS__)
#define __V4D__FE_53(__FE, X, ...) __FE(X)__V4D__FE_52(__FE, __VA_ARGS__)
#define __V4D__FE_54(__FE, X, ...) __FE(X)__V4D__FE_53(__FE, __VA_ARGS__)
#define __V4D__FE_55(__FE, X, ...) __FE(X)__V4D__FE_54(__FE, __VA_ARGS__)
#define __V4D__FE_56(__FE, X, ...) __FE(X)__V4D__FE_55(__FE, __VA_ARGS__)
#define __V4D__FE_57(__FE, X, ...) __FE(X)__V4D__FE_56(__FE, __VA_ARGS__)
#define __V4D__FE_58(__FE, X, ...) __FE(X)__V4D__FE_57(__FE, __VA_ARGS__)
#define __V4D__FE_59(__FE, X, ...) __FE(X)__V4D__FE_58(__FE, __VA_ARGS__)
#define __V4D__FE_60(__FE, X, ...) __FE(X)__V4D__FE_59(__FE, __VA_ARGS__)
#define __V4D__FE_61(__FE, X, ...) __FE(X)__V4D__FE_60(__FE, __VA_ARGS__)
#define __V4D__FE_62(__FE, X, ...) __FE(X)__V4D__FE_61(__FE, __VA_ARGS__)
#define __V4D__FE_63(__FE, X, ...) __FE(X)__V4D__FE_62(__FE, __VA_ARGS__)
#define __V4D__FE_64(__FE, X, ...) __FE(X)__V4D__FE_63(__FE, __VA_ARGS__)
#define __V4D__FE_65(__FE, X, ...) __FE(X)__V4D__FE_64(__FE, __VA_ARGS__)
#define __V4D__FE_66(__FE, X, ...) __FE(X)__V4D__FE_65(__FE, __VA_ARGS__)
#define __V4D__FE_67(__FE, X, ...) __FE(X)__V4D__FE_66(__FE, __VA_ARGS__)
#define __V4D__FE_68(__FE, X, ...) __FE(X)__V4D__FE_67(__FE, __VA_ARGS__)
#define __V4D__FE_69(__FE, X, ...) __FE(X)__V4D__FE_68(__FE, __VA_ARGS__)
#define __V4D__FE_70(__FE, X, ...) __FE(X)__V4D__FE_69(__FE, __VA_ARGS__)
#define __V4D__FE_71(__FE, X, ...) __FE(X)__V4D__FE_70(__FE, __VA_ARGS__)
#define __V4D__FE_72(__FE, X, ...) __FE(X)__V4D__FE_71(__FE, __VA_ARGS__)
#define __V4D__FE_73(__FE, X, ...) __FE(X)__V4D__FE_72(__FE, __VA_ARGS__)
#define __V4D__FE_74(__FE, X, ...) __FE(X)__V4D__FE_73(__FE, __VA_ARGS__)
#define __V4D__FE_75(__FE, X, ...) __FE(X)__V4D__FE_74(__FE, __VA_ARGS__)
#define __V4D__FE_76(__FE, X, ...) __FE(X)__V4D__FE_75(__FE, __VA_ARGS__)
#define __V4D__FE_77(__FE, X, ...) __FE(X)__V4D__FE_76(__FE, __VA_ARGS__)
#define __V4D__FE_78(__FE, X, ...) __FE(X)__V4D__FE_77(__FE, __VA_ARGS__)
#define __V4D__FE_79(__FE, X, ...) __FE(X)__V4D__FE_78(__FE, __VA_ARGS__)
#define __V4D__FE_80(__FE, X, ...) __FE(X)__V4D__FE_79(__FE, __VA_ARGS__)
#define __V4D__FE_81(__FE, X, ...) __FE(X)__V4D__FE_80(__FE, __VA_ARGS__)
#define __V4D__FE_82(__FE, X, ...) __FE(X)__V4D__FE_81(__FE, __VA_ARGS__)
#define __V4D__FE_83(__FE, X, ...) __FE(X)__V4D__FE_82(__FE, __VA_ARGS__)
#define __V4D__FE_84(__FE, X, ...) __FE(X)__V4D__FE_83(__FE, __VA_ARGS__)
#define __V4D__FE_85(__FE, X, ...) __FE(X)__V4D__FE_84(__FE, __VA_ARGS__)
#define __V4D__FE_86(__FE, X, ...) __FE(X)__V4D__FE_85(__FE, __VA_ARGS__)
#define __V4D__FE_87(__FE, X, ...) __FE(X)__V4D__FE_86(__FE, __VA_ARGS__)
#define __V4D__FE_88(__FE, X, ...) __FE(X)__V4D__FE_87(__FE, __VA_ARGS__)
#define __V4D__FE_89(__FE, X, ...) __FE(X)__V4D__FE_88(__FE, __VA_ARGS__)
#define __V4D__FE_90(__FE, X, ...) __FE(X)__V4D__FE_89(__FE, __VA_ARGS__)
#define __V4D__FE_91(__FE, X, ...) __FE(X)__V4D__FE_90(__FE, __VA_ARGS__)
#define __V4D__FE_92(__FE, X, ...) __FE(X)__V4D__FE_91(__FE, __VA_ARGS__)
#define __V4D__FE_93(__FE, X, ...) __FE(X)__V4D__FE_92(__FE, __VA_ARGS__)
#define __V4D__FE_94(__FE, X, ...) __FE(X)__V4D__FE_93(__FE, __VA_ARGS__)
#define __V4D__FE_95(__FE, X, ...) __FE(X)__V4D__FE_94(__FE, __VA_ARGS__)
#define __V4D__FE_96(__FE, X, ...) __FE(X)__V4D__FE_95(__FE, __VA_ARGS__)
#define __V4D__FE_97(__FE, X, ...) __FE(X)__V4D__FE_96(__FE, __VA_ARGS__)
#define __V4D__FE_98(__FE, X, ...) __FE(X)__V4D__FE_97(__FE, __VA_ARGS__)
#define __V4D__FE_99(__FE, X, ...) __FE(X)__V4D__FE_98(__FE, __VA_ARGS__)
#define __V4D__FE_100(__FE, X, ...) __FE(X)__V4D__FE_99(__FE, __VA_ARGS__)
#define __V4D__FE_101(__FE, X, ...) __FE(X)__V4D__FE_100(__FE, __VA_ARGS__)
#define __V4D__FE_102(__FE, X, ...) __FE(X)__V4D__FE_101(__FE, __VA_ARGS__)
#define __V4D__FE_103(__FE, X, ...) __FE(X)__V4D__FE_102(__FE, __VA_ARGS__)
#define __V4D__FE_104(__FE, X, ...) __FE(X)__V4D__FE_103(__FE, __VA_ARGS__)
#define __V4D__FE_105(__FE, X, ...) __FE(X)__V4D__FE_104(__FE, __VA_ARGS__)
#define __V4D__FE_106(__FE, X, ...) __FE(X)__V4D__FE_105(__FE, __VA_ARGS__)
#define __V4D__FE_107(__FE, X, ...) __FE(X)__V4D__FE_106(__FE, __VA_ARGS__)
#define __V4D__FE_108(__FE, X, ...) __FE(X)__V4D__FE_107(__FE, __VA_ARGS__)
#define __V4D__FE_109(__FE, X, ...) __FE(X)__V4D__FE_108(__FE, __VA_ARGS__)
#define __V4D__FE_110(__FE, X, ...) __FE(X)__V4D__FE_109(__FE, __VA_ARGS__)
#define __V4D__FE_111(__FE, X, ...) __FE(X)__V4D__FE_110(__FE, __VA_ARGS__)
#define __V4D__FE_112(__FE, X, ...) __FE(X)__V4D__FE_111(__FE, __VA_ARGS__)
#define __V4D__FE_113(__FE, X, ...) __FE(X)__V4D__FE_112(__FE, __VA_ARGS__)
#define __V4D__FE_114(__FE, X, ...) __FE(X)__V4D__FE_113(__FE, __VA_ARGS__)
#define __V4D__FE_115(__FE, X, ...) __FE(X)__V4D__FE_114(__FE, __VA_ARGS__)
#define __V4D__FE_116(__FE, X, ...) __FE(X)__V4D__FE_115(__FE, __VA_ARGS__)
#define __V4D__FE_117(__FE, X, ...) __FE(X)__V4D__FE_116(__FE, __VA_ARGS__)
#define __V4D__FE_118(__FE, X, ...) __FE(X)__V4D__FE_117(__FE, __VA_ARGS__)
#define __V4D__FE_119(__FE, X, ...) __FE(X)__V4D__FE_118(__FE, __VA_ARGS__)
#define __V4D__FE_120(__FE, X, ...) __FE(X)__V4D__FE_119(__FE, __VA_ARGS__)
#define __V4D__FE_121(__FE, X, ...) __FE(X)__V4D__FE_120(__FE, __VA_ARGS__)
#define __V4D__FE_122(__FE, X, ...) __FE(X)__V4D__FE_121(__FE, __VA_ARGS__)
#define __V4D__FE_123(__FE, X, ...) __FE(X)__V4D__FE_122(__FE, __VA_ARGS__)
#define __V4D__FE_124(__FE, X, ...) __FE(X)__V4D__FE_123(__FE, __VA_ARGS__)
#define __V4D__FE_125(__FE, X, ...) __FE(X)__V4D__FE_124(__FE, __VA_ARGS__)
#define __V4D__FE_126(__FE, X, ...) __FE(X)__V4D__FE_125(__FE, __VA_ARGS__)
#define __V4D__FE_127(__FE, X, ...) __FE(X)__V4D__FE_126(__FE, __VA_ARGS__)
#define __V4D__FE_128(__FE, X, ...) __FE(X)__V4D__FE_127(__FE, __VA_ARGS__)
#define __V4D__FE_129(__FE, X, ...) __FE(X)__V4D__FE_128(__FE, __VA_ARGS__)
#define __V4D__FE_130(__FE, X, ...) __FE(X)__V4D__FE_129(__FE, __VA_ARGS__)
#define __V4D__FE_131(__FE, X, ...) __FE(X)__V4D__FE_130(__FE, __VA_ARGS__)
#define __V4D__FE_132(__FE, X, ...) __FE(X)__V4D__FE_131(__FE, __VA_ARGS__)
#define __V4D__FE_133(__FE, X, ...) __FE(X)__V4D__FE_132(__FE, __VA_ARGS__)
#define __V4D__FE_134(__FE, X, ...) __FE(X)__V4D__FE_133(__FE, __VA_ARGS__)
#define __V4D__FE_135(__FE, X, ...) __FE(X)__V4D__FE_134(__FE, __VA_ARGS__)
#define __V4D__FE_136(__FE, X, ...) __FE(X)__V4D__FE_135(__FE, __VA_ARGS__)
#define __V4D__FE_137(__FE, X, ...) __FE(X)__V4D__FE_136(__FE, __VA_ARGS__)
#define __V4D__FE_138(__FE, X, ...) __FE(X)__V4D__FE_137(__FE, __VA_ARGS__)
#define __V4D__FE_139(__FE, X, ...) __FE(X)__V4D__FE_138(__FE, __VA_ARGS__)
#define __V4D__FE_140(__FE, X, ...) __FE(X)__V4D__FE_139(__FE, __VA_ARGS__)
#define __V4D__FE_141(__FE, X, ...) __FE(X)__V4D__FE_140(__FE, __VA_ARGS__)
#define __V4D__FE_142(__FE, X, ...) __FE(X)__V4D__FE_141(__FE, __VA_ARGS__)
#define __V4D__FE_143(__FE, X, ...) __FE(X)__V4D__FE_142(__FE, __VA_ARGS__)
#define __V4D__FE_144(__FE, X, ...) __FE(X)__V4D__FE_143(__FE, __VA_ARGS__)
#define __V4D__FE_145(__FE, X, ...) __FE(X)__V4D__FE_144(__FE, __VA_ARGS__)
#define __V4D__FE_146(__FE, X, ...) __FE(X)__V4D__FE_145(__FE, __VA_ARGS__)
#define __V4D__FE_147(__FE, X, ...) __FE(X)__V4D__FE_146(__FE, __VA_ARGS__)
#define __V4D__FE_148(__FE, X, ...) __FE(X)__V4D__FE_147(__FE, __VA_ARGS__)
#define __V4D__FE_149(__FE, X, ...) __FE(X)__V4D__FE_148(__FE, __VA_ARGS__)
#define __V4D__FE_150(__FE, X, ...) __FE(X)__V4D__FE_149(__FE, __VA_ARGS__)
#define __V4D__FE_151(__FE, X, ...) __FE(X)__V4D__FE_150(__FE, __VA_ARGS__)
#define __V4D__FE_152(__FE, X, ...) __FE(X)__V4D__FE_151(__FE, __VA_ARGS__)
#define __V4D__FE_153(__FE, X, ...) __FE(X)__V4D__FE_152(__FE, __VA_ARGS__)
#define __V4D__FE_154(__FE, X, ...) __FE(X)__V4D__FE_153(__FE, __VA_ARGS__)
#define __V4D__FE_155(__FE, X, ...) __FE(X)__V4D__FE_154(__FE, __VA_ARGS__)
#define __V4D__FE_156(__FE, X, ...) __FE(X)__V4D__FE_155(__FE, __VA_ARGS__)
#define __V4D__FE_157(__FE, X, ...) __FE(X)__V4D__FE_156(__FE, __VA_ARGS__)
#define __V4D__FE_158(__FE, X, ...) __FE(X)__V4D__FE_157(__FE, __VA_ARGS__)
#define __V4D__FE_159(__FE, X, ...) __FE(X)__V4D__FE_158(__FE, __VA_ARGS__)
#define __V4D__FE_160(__FE, X, ...) __FE(X)__V4D__FE_159(__FE, __VA_ARGS__)
#define __V4D__FE_161(__FE, X, ...) __FE(X)__V4D__FE_160(__FE, __VA_ARGS__)
#define __V4D__FE_162(__FE, X, ...) __FE(X)__V4D__FE_161(__FE, __VA_ARGS__)
#define __V4D__FE_163(__FE, X, ...) __FE(X)__V4D__FE_162(__FE, __VA_ARGS__)
#define __V4D__FE_164(__FE, X, ...) __FE(X)__V4D__FE_163(__FE, __VA_ARGS__)
#define __V4D__FE_165(__FE, X, ...) __FE(X)__V4D__FE_164(__FE, __VA_ARGS__)
#define __V4D__FE_166(__FE, X, ...) __FE(X)__V4D__FE_165(__FE, __VA_ARGS__)
#define __V4D__FE_167(__FE, X, ...) __FE(X)__V4D__FE_166(__FE, __VA_ARGS__)
#define __V4D__FE_168(__FE, X, ...) __FE(X)__V4D__FE_167(__FE, __VA_ARGS__)
#define __V4D__FE_169(__FE, X, ...) __FE(X)__V4D__FE_168(__FE, __VA_ARGS__)
#define __V4D__FE_170(__FE, X, ...) __FE(X)__V4D__FE_169(__FE, __VA_ARGS__)
#define __V4D__FE_171(__FE, X, ...) __FE(X)__V4D__FE_170(__FE, __VA_ARGS__)
#define __V4D__FE_172(__FE, X, ...) __FE(X)__V4D__FE_171(__FE, __VA_ARGS__)
#define __V4D__FE_173(__FE, X, ...) __FE(X)__V4D__FE_172(__FE, __VA_ARGS__)
#define __V4D__FE_174(__FE, X, ...) __FE(X)__V4D__FE_173(__FE, __VA_ARGS__)
#define __V4D__FE_175(__FE, X, ...) __FE(X)__V4D__FE_174(__FE, __VA_ARGS__)
#define __V4D__FE_176(__FE, X, ...) __FE(X)__V4D__FE_175(__FE, __VA_ARGS__)
#define __V4D__FE_177(__FE, X, ...) __FE(X)__V4D__FE_176(__FE, __VA_ARGS__)
#define __V4D__FE_178(__FE, X, ...) __FE(X)__V4D__FE_177(__FE, __VA_ARGS__)
#define __V4D__FE_179(__FE, X, ...) __FE(X)__V4D__FE_178(__FE, __VA_ARGS__)
#define __V4D__FE_180(__FE, X, ...) __FE(X)__V4D__FE_179(__FE, __VA_ARGS__)
#define __V4D__FE_181(__FE, X, ...) __FE(X)__V4D__FE_180(__FE, __VA_ARGS__)
#define __V4D__FE_182(__FE, X, ...) __FE(X)__V4D__FE_181(__FE, __VA_ARGS__)
#define __V4D__FE_183(__FE, X, ...) __FE(X)__V4D__FE_182(__FE, __VA_ARGS__)
#define __V4D__FE_184(__FE, X, ...) __FE(X)__V4D__FE_183(__FE, __VA_ARGS__)
#define __V4D__FE_185(__FE, X, ...) __FE(X)__V4D__FE_184(__FE, __VA_ARGS__)
#define __V4D__FE_186(__FE, X, ...) __FE(X)__V4D__FE_185(__FE, __VA_ARGS__)
#define __V4D__FE_187(__FE, X, ...) __FE(X)__V4D__FE_186(__FE, __VA_ARGS__)
#define __V4D__FE_188(__FE, X, ...) __FE(X)__V4D__FE_187(__FE, __VA_ARGS__)
#define __V4D__FE_189(__FE, X, ...) __FE(X)__V4D__FE_188(__FE, __VA_ARGS__)
#define __V4D__FE_190(__FE, X, ...) __FE(X)__V4D__FE_189(__FE, __VA_ARGS__)
#define __V4D__FE_191(__FE, X, ...) __FE(X)__V4D__FE_190(__FE, __VA_ARGS__)
#define __V4D__FE_192(__FE, X, ...) __FE(X)__V4D__FE_191(__FE, __VA_ARGS__)
#define __V4D__FE_193(__FE, X, ...) __FE(X)__V4D__FE_192(__FE, __VA_ARGS__)
#define __V4D__FE_194(__FE, X, ...) __FE(X)__V4D__FE_193(__FE, __VA_ARGS__)
#define __V4D__FE_195(__FE, X, ...) __FE(X)__V4D__FE_194(__FE, __VA_ARGS__)
#define __V4D__FE_196(__FE, X, ...) __FE(X)__V4D__FE_195(__FE, __VA_ARGS__)
#define __V4D__FE_197(__FE, X, ...) __FE(X)__V4D__FE_196(__FE, __VA_ARGS__)
#define __V4D__FE_198(__FE, X, ...) __FE(X)__V4D__FE_197(__FE, __VA_ARGS__)
#define __V4D__FE_199(__FE, X, ...) __FE(X)__V4D__FE_198(__FE, __VA_ARGS__)
#define __V4D__FE_200(__FE, X, ...) __FE(X)__V4D__FE_199(__FE, __VA_ARGS__)
#define __V4D__FE_201(__FE, X, ...) __FE(X)__V4D__FE_200(__FE, __VA_ARGS__)
#define __V4D__FE_202(__FE, X, ...) __FE(X)__V4D__FE_201(__FE, __VA_ARGS__)
#define __V4D__FE_203(__FE, X, ...) __FE(X)__V4D__FE_202(__FE, __VA_ARGS__)
#define __V4D__FE_204(__FE, X, ...) __FE(X)__V4D__FE_203(__FE, __VA_ARGS__)
#define __V4D__FE_205(__FE, X, ...) __FE(X)__V4D__FE_204(__FE, __VA_ARGS__)
#define __V4D__FE_206(__FE, X, ...) __FE(X)__V4D__FE_205(__FE, __VA_ARGS__)
#define __V4D__FE_207(__FE, X, ...) __FE(X)__V4D__FE_206(__FE, __VA_ARGS__)
#define __V4D__FE_208(__FE, X, ...) __FE(X)__V4D__FE_207(__FE, __VA_ARGS__)
#define __V4D__FE_209(__FE, X, ...) __FE(X)__V4D__FE_208(__FE, __VA_ARGS__)
#define __V4D__FE_210(__FE, X, ...) __FE(X)__V4D__FE_209(__FE, __VA_ARGS__)
#define __V4D__FE_211(__FE, X, ...) __FE(X)__V4D__FE_210(__FE, __VA_ARGS__)
#define __V4D__FE_212(__FE, X, ...) __FE(X)__V4D__FE_211(__FE, __VA_ARGS__)
#define __V4D__FE_213(__FE, X, ...) __FE(X)__V4D__FE_212(__FE, __VA_ARGS__)
#define __V4D__FE_214(__FE, X, ...) __FE(X)__V4D__FE_213(__FE, __VA_ARGS__)
#define __V4D__FE_215(__FE, X, ...) __FE(X)__V4D__FE_214(__FE, __VA_ARGS__)
#define __V4D__FE_216(__FE, X, ...) __FE(X)__V4D__FE_215(__FE, __VA_ARGS__)
#define __V4D__FE_217(__FE, X, ...) __FE(X)__V4D__FE_216(__FE, __VA_ARGS__)
#define __V4D__FE_218(__FE, X, ...) __FE(X)__V4D__FE_217(__FE, __VA_ARGS__)
#define __V4D__FE_219(__FE, X, ...) __FE(X)__V4D__FE_218(__FE, __VA_ARGS__)
#define __V4D__FE_220(__FE, X, ...) __FE(X)__V4D__FE_219(__FE, __VA_ARGS__)
#define __V4D__FE_221(__FE, X, ...) __FE(X)__V4D__FE_220(__FE, __VA_ARGS__)
#define __V4D__FE_222(__FE, X, ...) __FE(X)__V4D__FE_221(__FE, __VA_ARGS__)
#define __V4D__FE_223(__FE, X, ...) __FE(X)__V4D__FE_222(__FE, __VA_ARGS__)
#define __V4D__FE_224(__FE, X, ...) __FE(X)__V4D__FE_223(__FE, __VA_ARGS__)
#define __V4D__FE_225(__FE, X, ...) __FE(X)__V4D__FE_224(__FE, __VA_ARGS__)
#define __V4D__FE_226(__FE, X, ...) __FE(X)__V4D__FE_225(__FE, __VA_ARGS__)
#define __V4D__FE_227(__FE, X, ...) __FE(X)__V4D__FE_226(__FE, __VA_ARGS__)
#define __V4D__FE_228(__FE, X, ...) __FE(X)__V4D__FE_227(__FE, __VA_ARGS__)
#define __V4D__FE_229(__FE, X, ...) __FE(X)__V4D__FE_228(__FE, __VA_ARGS__)
#define __V4D__FE_230(__FE, X, ...) __FE(X)__V4D__FE_229(__FE, __VA_ARGS__)
#define __V4D__FE_231(__FE, X, ...) __FE(X)__V4D__FE_230(__FE, __VA_ARGS__)
#define __V4D__FE_232(__FE, X, ...) __FE(X)__V4D__FE_231(__FE, __VA_ARGS__)
#define __V4D__FE_233(__FE, X, ...) __FE(X)__V4D__FE_232(__FE, __VA_ARGS__)
#define __V4D__FE_234(__FE, X, ...) __FE(X)__V4D__FE_233(__FE, __VA_ARGS__)
#define __V4D__FE_235(__FE, X, ...) __FE(X)__V4D__FE_234(__FE, __VA_ARGS__)
#define __V4D__FE_236(__FE, X, ...) __FE(X)__V4D__FE_235(__FE, __VA_ARGS__)
#define __V4D__FE_237(__FE, X, ...) __FE(X)__V4D__FE_236(__FE, __VA_ARGS__)
#define __V4D__FE_238(__FE, X, ...) __FE(X)__V4D__FE_237(__FE, __VA_ARGS__)
#define __V4D__FE_239(__FE, X, ...) __FE(X)__V4D__FE_238(__FE, __VA_ARGS__)
#define __V4D__FE_240(__FE, X, ...) __FE(X)__V4D__FE_239(__FE, __VA_ARGS__)
#define __V4D__FE_241(__FE, X, ...) __FE(X)__V4D__FE_240(__FE, __VA_ARGS__)
#define __V4D__FE_242(__FE, X, ...) __FE(X)__V4D__FE_241(__FE, __VA_ARGS__)
#define __V4D__FE_243(__FE, X, ...) __FE(X)__V4D__FE_242(__FE, __VA_ARGS__)
#define __V4D__FE_244(__FE, X, ...) __FE(X)__V4D__FE_243(__FE, __VA_ARGS__)
#define __V4D__FE_245(__FE, X, ...) __FE(X)__V4D__FE_244(__FE, __VA_ARGS__)
#define __V4D__FE_246(__FE, X, ...) __FE(X)__V4D__FE_245(__FE, __VA_ARGS__)
#define __V4D__FE_247(__FE, X, ...) __FE(X)__V4D__FE_246(__FE, __VA_ARGS__)
#define __V4D__FE_248(__FE, X, ...) __FE(X)__V4D__FE_247(__FE, __VA_ARGS__)
#define __V4D__FE_249(__FE, X, ...) __FE(X)__V4D__FE_248(__FE, __VA_ARGS__)
#define __V4D__FE_250(__FE, X, ...) __FE(X)__V4D__FE_249(__FE, __VA_ARGS__)
#define __V4D__FE_251(__FE, X, ...) __FE(X)__V4D__FE_250(__FE, __VA_ARGS__)
#define __V4D__FE_252(__FE, X, ...) __FE(X)__V4D__FE_251(__FE, __VA_ARGS__)
#define __V4D__FE_253(__FE, X, ...) __FE(X)__V4D__FE_252(__FE, __VA_ARGS__)
#define __V4D__FE_254(__FE, X, ...) __FE(X)__V4D__FE_253(__FE, __VA_ARGS__)
#define __V4D__FE_255(__FE, X, ...) __FE(X)__V4D__FE_254(__FE, __VA_ARGS__)
#define __V4D__FE_256(__FE, X, ...) __FE(X)__V4D__FE_255(__FE, __VA_ARGS__)

#define __V4D__FOR_EACH(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,_128,_129,_130,_131,_132,_133,_134,_135,_136,_137,_138,_139,_140,_141,_142,_143,_144,_145,_146,_147,_148,_149,_150,_151,_152,_153,_154,_155,_156,_157,_158,_159,_160,_161,_162,_163,_164,_165,_166,_167,_168,_169,_170,_171,_172,_173,_174,_175,_176,_177,_178,_179,_180,_181,_182,_183,_184,_185,_186,_187,_188,_189,_190,_191,_192,_193,_194,_195,_196,_197,_198,_199,_200,_201,_202,_203,_204,_205,_206,_207,_208,_209,_210,_211,_212,_213,_214,_215,_216,_217,_218,_219,_220,_221,_222,_223,_224,_225,_226,_227,_228,_229,_230,_231,_232,_233,_234,_235,_236,_237,_238,_239,_240,_241,_242,_243,_244,_245,_246,_247,_248,_249,_250,_251,_252,_253,_254,_255,_256,macroName,...) macroName 
#define FOR_EACH(_MACRO,...) __V4D__FOR_EACH(__VA_ARGS__,__V4D__FE_256,__V4D__FE_255,__V4D__FE_254,__V4D__FE_253,__V4D__FE_252,__V4D__FE_251,__V4D__FE_250,__V4D__FE_249,__V4D__FE_248,__V4D__FE_247,__V4D__FE_246,__V4D__FE_245,__V4D__FE_244,__V4D__FE_243,__V4D__FE_242,__V4D__FE_241,__V4D__FE_240,__V4D__FE_239,__V4D__FE_238,__V4D__FE_237,__V4D__FE_236,__V4D__FE_235,__V4D__FE_234,__V4D__FE_233,__V4D__FE_232,__V4D__FE_231,__V4D__FE_230,__V4D__FE_229,__V4D__FE_228,__V4D__FE_227,__V4D__FE_226,__V4D__FE_225,__V4D__FE_224,__V4D__FE_223,__V4D__FE_222,__V4D__FE_221,__V4D__FE_220,__V4D__FE_219,__V4D__FE_218,__V4D__FE_217,__V4D__FE_216,__V4D__FE_215,__V4D__FE_214,__V4D__FE_213,__V4D__FE_212,__V4D__FE_211,__V4D__FE_210,__V4D__FE_209,__V4D__FE_208,__V4D__FE_207,__V4D__FE_206,__V4D__FE_205,__V4D__FE_204,__V4D__FE_203,__V4D__FE_202,__V4D__FE_201,__V4D__FE_200,__V4D__FE_199,__V4D__FE_198,__V4D__FE_197,__V4D__FE_196,__V4D__FE_195,__V4D__FE_194,__V4D__FE_193,__V4D__FE_192,__V4D__FE_191,__V4D__FE_190,__V4D__FE_189,__V4D__FE_188,__V4D__FE_187,__V4D__FE_186,__V4D__FE_185,__V4D__FE_184,__V4D__FE_183,__V4D__FE_182,__V4D__FE_181,__V4D__FE_180,__V4D__FE_179,__V4D__FE_178,__V4D__FE_177,__V4D__FE_176,__V4D__FE_175,__V4D__FE_174,__V4D__FE_173,__V4D__FE_172,__V4D__FE_171,__V4D__FE_170,__V4D__FE_169,__V4D__FE_168,__V4D__FE_167,__V4D__FE_166,__V4D__FE_165,__V4D__FE_164,__V4D__FE_163,__V4D__FE_162,__V4D__FE_161,__V4D__FE_160,__V4D__FE_159,__V4D__FE_158,__V4D__FE_157,__V4D__FE_156,__V4D__FE_155,__V4D__FE_154,__V4D__FE_153,__V4D__FE_152,__V4D__FE_151,__V4D__FE_150,__V4D__FE_149,__V4D__FE_148,__V4D__FE_147,__V4D__FE_146,__V4D__FE_145,__V4D__FE_144,__V4D__FE_143,__V4D__FE_142,__V4D__FE_141,__V4D__FE_140,__V4D__FE_139,__V4D__FE_138,__V4D__FE_137,__V4D__FE_136,__V4D__FE_135,__V4D__FE_134,__V4D__FE_133,__V4D__FE_132,__V4D__FE_131,__V4D__FE_130,__V4D__FE_129,__V4D__FE_128,__V4D__FE_127,__V4D__FE_126,__V4D__FE_125,__V4D__FE_124,__V4D__FE_123,__V4D__FE_122,__V4D__FE_121,__V4D__FE_120,__V4D__FE_119,__V4D__FE_118,__V4D__FE_117,__V4D__FE_116,__V4D__FE_115,__V4D__FE_114,__V4D__FE_113,__V4D__FE_112,__V4D__FE_111,__V4D__FE_110,__V4D__FE_109,__V4D__FE_108,__V4D__FE_107,__V4D__FE_106,__V4D__FE_105,__V4D__FE_104,__V4D__FE_103,__V4D__FE_102,__V4D__FE_101,__V4D__FE_100,__V4D__FE_99,__V4D__FE_98,__V4D__FE_97,__V4D__FE_96,__V4D__FE_95,__V4D__FE_94,__V4D__FE_93,__V4D__FE_92,__V4D__FE_91,__V4D__FE_90,__V4D__FE_89,__V4D__FE_88,__V4D__FE_87,__V4D__FE_86,__V4D__FE_85,__V4D__FE_84,__V4D__FE_83,__V4D__FE_82,__V4D__FE_81,__V4D__FE_80,__V4D__FE_79,__V4D__FE_78,__V4D__FE_77,__V4D__FE_76,__V4D__FE_75,__V4D__FE_74,__V4D__FE_73,__V4D__FE_72,__V4D__FE_71,__V4D__FE_70,__V4D__FE_69,__V4D__FE_68,__V4D__FE_67,__V4D__FE_66,__V4D__FE_65,__V4D__FE_64,__V4D__FE_63,__V4D__FE_62,__V4D__FE_61,__V4D__FE_60,__V4D__FE_59,__V4D__FE_58,__V4D__FE_57,__V4D__FE_56,__V4D__FE_55,__V4D__FE_54,__V4D__FE_53,__V4D__FE_52,__V4D__FE_51,__V4D__FE_50,__V4D__FE_49,__V4D__FE_48,__V4D__FE_47,__V4D__FE_46,__V4D__FE_45,__V4D__FE_44,__V4D__FE_43,__V4D__FE_42,__V4D__FE_41,__V4D__FE_40,__V4D__FE_39,__V4D__FE_38,__V4D__FE_37,__V4D__FE_36,__V4D__FE_35,__V4D__FE_34,__V4D__FE_33,__V4D__FE_32,__V4D__FE_31,__V4D__FE_30,__V4D__FE_29,__V4D__FE_28,__V4D__FE_27,__V4D__FE_26,__V4D__FE_25,__V4D__FE_24,__V4D__FE_23,__V4D__FE_22,__V4D__FE_21,__V4D__FE_20,__V4D__FE_19,__V4D__FE_18,__V4D__FE_17,__V4D__FE_16,__V4D__FE_15,__V4D__FE_14,__V4D__FE_13,__V4D__FE_12,__V4D__FE_11,__V4D__FE_10,__V4D__FE_9,__V4D__FE_8,__V4D__FE_7,__V4D__FE_6,__V4D__FE_5,__V4D__FE_4,__V4D__FE_3,__V4D__FE_2,__V4D__FE_1)(_MACRO,__VA_ARGS__)


//////////////////////////////////////////////////////////
// CPU Affinity

#ifdef _WINDOWS
	#define __V4D__CPU_AFFINITY_ADD___(n) processAffinityMask |= (1 << ((n) % std::thread::hardware_concurrency()));
	#define SET_CPU_AFFINITY(...) {\
		DWORD_PTR processAffinityMask = 0;\
		FOR_EACH(__V4D__CPU_AFFINITY_ADD___, __VA_ARGS__)\
		if (!SetThreadAffinityMask(GetCurrentThread(), processAffinityMask)) LOG_ERROR("Error calling SetThreadAffinityMask");\
	}
	#define __V4D__CPU_AFFINITY_REMOVE___(n) processAffinityMask &= ~(1 << ((n) % std::thread::hardware_concurrency()));
	#define UNSET_CPU_AFFINITY(...) {\
		DWORD_PTR processAffinityMask = 0;\
		for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) __V4D__CPU_AFFINITY_ADD___(i)\
		FOR_EACH(__V4D__CPU_AFFINITY_REMOVE___, __VA_ARGS__)\
		if (!SetThreadAffinityMask(GetCurrentThread(), processAffinityMask)) LOG_ERROR("Error calling SetThreadAffinityMask");\
	}
	#define SET_THREAD_HIGHEST_PRIORITY() {\
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);\
	}
#else
	#define __V4D__CPU_AFFINITY_ADD___(n) CPU_SET((n) % std::thread::hardware_concurrency(), &cpuset);
	#define SET_CPU_AFFINITY(...) {\
		cpu_set_t cpuset;\
		CPU_ZERO(&cpuset);\
		FOR_EACH(__V4D__CPU_AFFINITY_ADD___, __VA_ARGS__)\
		int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);\
		if (rc != 0) LOG_ERROR("Error calling pthread_setaffinity_np: error " << std::strerror(rc))\
	}
	#define __V4D__CPU_AFFINITY_REMOVE___(n) CPU_CLR((n) % std::thread::hardware_concurrency(), &cpuset);
	#define UNSET_CPU_AFFINITY(...) {\
		cpu_set_t cpuset;\
		for (int i = 0; i < std::thread::hardware_concurrency(); ++i) __V4D__CPU_AFFINITY_ADD___(i)\
		FOR_EACH(__V4D__CPU_AFFINITY_REMOVE___, __VA_ARGS__)\
		int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);\
		if (rc != 0) LOG_ERROR("Error calling pthread_setaffinity_np: error " << std::strerror(rc))\
	}
	#define SET_THREAD_HIGHEST_PRIORITY() {\
		pthread_attr_t thAttr;\
		int policy = 0;\
		pthread_attr_init(&thAttr);\
		pthread_attr_getschedpolicy(&thAttr, &policy);\
		int max_prio_for_policy = sched_get_priority_max(policy);\
		pthread_setschedprio(pthread_self(), max_prio_for_policy);\
		pthread_attr_destroy(&thAttr);\
	}
#endif


//////////////////////////////////////////////////////////
// Threads

#define THREAD_ID_STR std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()))

#define THREAD_YIELD {std::this_thread::yield();SLEEP(1ms)}

// Concepts
#define REQUIRES_HAS_FUNC(T, FuncName, ReturnType) requires(T){{T::FuncName()} -> std::convertible_to<ReturnType>; };

