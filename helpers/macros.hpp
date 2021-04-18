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
	#define V4DGAME
#else// Project/Module
	#define V4DLIB DLLIMPORT
	#ifdef _V4D_GAME
		#define V4DGAME DLLEXPORT
	#else// Project/Module
		#define V4DGAME DLLIMPORT
	#endif
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
#define STATIC_CLASS_INSTANCES_CPP(key, className, ...) { \
	static std::mutex staticMu; \
	std::lock_guard staticLock(staticMu); \
	static std::unordered_map<std::decay<decltype(key)>::type, std::shared_ptr<className>> instances {}; \
	if (instances.count(key) == 0) { \
		instances.emplace(key, std::make_shared<className>(__VA_ARGS__)); \
	} \
	return instances.at(key); \
}


//////////////////////////////////////////////////////////
// FOREACH macro (maximum of 256 arguments, may increase in the future if needed)

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
#define __FE_65(__FE, X, ...) __FE(X)__FE_64(__FE, __VA_ARGS__)
#define __FE_66(__FE, X, ...) __FE(X)__FE_65(__FE, __VA_ARGS__)
#define __FE_67(__FE, X, ...) __FE(X)__FE_66(__FE, __VA_ARGS__)
#define __FE_68(__FE, X, ...) __FE(X)__FE_67(__FE, __VA_ARGS__)
#define __FE_69(__FE, X, ...) __FE(X)__FE_68(__FE, __VA_ARGS__)
#define __FE_70(__FE, X, ...) __FE(X)__FE_69(__FE, __VA_ARGS__)
#define __FE_71(__FE, X, ...) __FE(X)__FE_70(__FE, __VA_ARGS__)
#define __FE_72(__FE, X, ...) __FE(X)__FE_71(__FE, __VA_ARGS__)
#define __FE_73(__FE, X, ...) __FE(X)__FE_72(__FE, __VA_ARGS__)
#define __FE_74(__FE, X, ...) __FE(X)__FE_73(__FE, __VA_ARGS__)
#define __FE_75(__FE, X, ...) __FE(X)__FE_74(__FE, __VA_ARGS__)
#define __FE_76(__FE, X, ...) __FE(X)__FE_75(__FE, __VA_ARGS__)
#define __FE_77(__FE, X, ...) __FE(X)__FE_76(__FE, __VA_ARGS__)
#define __FE_78(__FE, X, ...) __FE(X)__FE_77(__FE, __VA_ARGS__)
#define __FE_79(__FE, X, ...) __FE(X)__FE_78(__FE, __VA_ARGS__)
#define __FE_80(__FE, X, ...) __FE(X)__FE_79(__FE, __VA_ARGS__)
#define __FE_81(__FE, X, ...) __FE(X)__FE_80(__FE, __VA_ARGS__)
#define __FE_82(__FE, X, ...) __FE(X)__FE_81(__FE, __VA_ARGS__)
#define __FE_83(__FE, X, ...) __FE(X)__FE_82(__FE, __VA_ARGS__)
#define __FE_84(__FE, X, ...) __FE(X)__FE_83(__FE, __VA_ARGS__)
#define __FE_85(__FE, X, ...) __FE(X)__FE_84(__FE, __VA_ARGS__)
#define __FE_86(__FE, X, ...) __FE(X)__FE_85(__FE, __VA_ARGS__)
#define __FE_87(__FE, X, ...) __FE(X)__FE_86(__FE, __VA_ARGS__)
#define __FE_88(__FE, X, ...) __FE(X)__FE_87(__FE, __VA_ARGS__)
#define __FE_89(__FE, X, ...) __FE(X)__FE_88(__FE, __VA_ARGS__)
#define __FE_90(__FE, X, ...) __FE(X)__FE_89(__FE, __VA_ARGS__)
#define __FE_91(__FE, X, ...) __FE(X)__FE_90(__FE, __VA_ARGS__)
#define __FE_92(__FE, X, ...) __FE(X)__FE_91(__FE, __VA_ARGS__)
#define __FE_93(__FE, X, ...) __FE(X)__FE_92(__FE, __VA_ARGS__)
#define __FE_94(__FE, X, ...) __FE(X)__FE_93(__FE, __VA_ARGS__)
#define __FE_95(__FE, X, ...) __FE(X)__FE_94(__FE, __VA_ARGS__)
#define __FE_96(__FE, X, ...) __FE(X)__FE_95(__FE, __VA_ARGS__)
#define __FE_97(__FE, X, ...) __FE(X)__FE_96(__FE, __VA_ARGS__)
#define __FE_98(__FE, X, ...) __FE(X)__FE_97(__FE, __VA_ARGS__)
#define __FE_99(__FE, X, ...) __FE(X)__FE_98(__FE, __VA_ARGS__)
#define __FE_100(__FE, X, ...) __FE(X)__FE_99(__FE, __VA_ARGS__)
#define __FE_101(__FE, X, ...) __FE(X)__FE_100(__FE, __VA_ARGS__)
#define __FE_102(__FE, X, ...) __FE(X)__FE_101(__FE, __VA_ARGS__)
#define __FE_103(__FE, X, ...) __FE(X)__FE_102(__FE, __VA_ARGS__)
#define __FE_104(__FE, X, ...) __FE(X)__FE_103(__FE, __VA_ARGS__)
#define __FE_105(__FE, X, ...) __FE(X)__FE_104(__FE, __VA_ARGS__)
#define __FE_106(__FE, X, ...) __FE(X)__FE_105(__FE, __VA_ARGS__)
#define __FE_107(__FE, X, ...) __FE(X)__FE_106(__FE, __VA_ARGS__)
#define __FE_108(__FE, X, ...) __FE(X)__FE_107(__FE, __VA_ARGS__)
#define __FE_109(__FE, X, ...) __FE(X)__FE_108(__FE, __VA_ARGS__)
#define __FE_110(__FE, X, ...) __FE(X)__FE_109(__FE, __VA_ARGS__)
#define __FE_111(__FE, X, ...) __FE(X)__FE_110(__FE, __VA_ARGS__)
#define __FE_112(__FE, X, ...) __FE(X)__FE_111(__FE, __VA_ARGS__)
#define __FE_113(__FE, X, ...) __FE(X)__FE_112(__FE, __VA_ARGS__)
#define __FE_114(__FE, X, ...) __FE(X)__FE_113(__FE, __VA_ARGS__)
#define __FE_115(__FE, X, ...) __FE(X)__FE_114(__FE, __VA_ARGS__)
#define __FE_116(__FE, X, ...) __FE(X)__FE_115(__FE, __VA_ARGS__)
#define __FE_117(__FE, X, ...) __FE(X)__FE_116(__FE, __VA_ARGS__)
#define __FE_118(__FE, X, ...) __FE(X)__FE_117(__FE, __VA_ARGS__)
#define __FE_119(__FE, X, ...) __FE(X)__FE_118(__FE, __VA_ARGS__)
#define __FE_120(__FE, X, ...) __FE(X)__FE_119(__FE, __VA_ARGS__)
#define __FE_121(__FE, X, ...) __FE(X)__FE_120(__FE, __VA_ARGS__)
#define __FE_122(__FE, X, ...) __FE(X)__FE_121(__FE, __VA_ARGS__)
#define __FE_123(__FE, X, ...) __FE(X)__FE_122(__FE, __VA_ARGS__)
#define __FE_124(__FE, X, ...) __FE(X)__FE_123(__FE, __VA_ARGS__)
#define __FE_125(__FE, X, ...) __FE(X)__FE_124(__FE, __VA_ARGS__)
#define __FE_126(__FE, X, ...) __FE(X)__FE_125(__FE, __VA_ARGS__)
#define __FE_127(__FE, X, ...) __FE(X)__FE_126(__FE, __VA_ARGS__)
#define __FE_128(__FE, X, ...) __FE(X)__FE_127(__FE, __VA_ARGS__)
#define __FE_129(__FE, X, ...) __FE(X)__FE_128(__FE, __VA_ARGS__)
#define __FE_130(__FE, X, ...) __FE(X)__FE_129(__FE, __VA_ARGS__)
#define __FE_131(__FE, X, ...) __FE(X)__FE_130(__FE, __VA_ARGS__)
#define __FE_132(__FE, X, ...) __FE(X)__FE_131(__FE, __VA_ARGS__)
#define __FE_133(__FE, X, ...) __FE(X)__FE_132(__FE, __VA_ARGS__)
#define __FE_134(__FE, X, ...) __FE(X)__FE_133(__FE, __VA_ARGS__)
#define __FE_135(__FE, X, ...) __FE(X)__FE_134(__FE, __VA_ARGS__)
#define __FE_136(__FE, X, ...) __FE(X)__FE_135(__FE, __VA_ARGS__)
#define __FE_137(__FE, X, ...) __FE(X)__FE_136(__FE, __VA_ARGS__)
#define __FE_138(__FE, X, ...) __FE(X)__FE_137(__FE, __VA_ARGS__)
#define __FE_139(__FE, X, ...) __FE(X)__FE_138(__FE, __VA_ARGS__)
#define __FE_140(__FE, X, ...) __FE(X)__FE_139(__FE, __VA_ARGS__)
#define __FE_141(__FE, X, ...) __FE(X)__FE_140(__FE, __VA_ARGS__)
#define __FE_142(__FE, X, ...) __FE(X)__FE_141(__FE, __VA_ARGS__)
#define __FE_143(__FE, X, ...) __FE(X)__FE_142(__FE, __VA_ARGS__)
#define __FE_144(__FE, X, ...) __FE(X)__FE_143(__FE, __VA_ARGS__)
#define __FE_145(__FE, X, ...) __FE(X)__FE_144(__FE, __VA_ARGS__)
#define __FE_146(__FE, X, ...) __FE(X)__FE_145(__FE, __VA_ARGS__)
#define __FE_147(__FE, X, ...) __FE(X)__FE_146(__FE, __VA_ARGS__)
#define __FE_148(__FE, X, ...) __FE(X)__FE_147(__FE, __VA_ARGS__)
#define __FE_149(__FE, X, ...) __FE(X)__FE_148(__FE, __VA_ARGS__)
#define __FE_150(__FE, X, ...) __FE(X)__FE_149(__FE, __VA_ARGS__)
#define __FE_151(__FE, X, ...) __FE(X)__FE_150(__FE, __VA_ARGS__)
#define __FE_152(__FE, X, ...) __FE(X)__FE_151(__FE, __VA_ARGS__)
#define __FE_153(__FE, X, ...) __FE(X)__FE_152(__FE, __VA_ARGS__)
#define __FE_154(__FE, X, ...) __FE(X)__FE_153(__FE, __VA_ARGS__)
#define __FE_155(__FE, X, ...) __FE(X)__FE_154(__FE, __VA_ARGS__)
#define __FE_156(__FE, X, ...) __FE(X)__FE_155(__FE, __VA_ARGS__)
#define __FE_157(__FE, X, ...) __FE(X)__FE_156(__FE, __VA_ARGS__)
#define __FE_158(__FE, X, ...) __FE(X)__FE_157(__FE, __VA_ARGS__)
#define __FE_159(__FE, X, ...) __FE(X)__FE_158(__FE, __VA_ARGS__)
#define __FE_160(__FE, X, ...) __FE(X)__FE_159(__FE, __VA_ARGS__)
#define __FE_161(__FE, X, ...) __FE(X)__FE_160(__FE, __VA_ARGS__)
#define __FE_162(__FE, X, ...) __FE(X)__FE_161(__FE, __VA_ARGS__)
#define __FE_163(__FE, X, ...) __FE(X)__FE_162(__FE, __VA_ARGS__)
#define __FE_164(__FE, X, ...) __FE(X)__FE_163(__FE, __VA_ARGS__)
#define __FE_165(__FE, X, ...) __FE(X)__FE_164(__FE, __VA_ARGS__)
#define __FE_166(__FE, X, ...) __FE(X)__FE_165(__FE, __VA_ARGS__)
#define __FE_167(__FE, X, ...) __FE(X)__FE_166(__FE, __VA_ARGS__)
#define __FE_168(__FE, X, ...) __FE(X)__FE_167(__FE, __VA_ARGS__)
#define __FE_169(__FE, X, ...) __FE(X)__FE_168(__FE, __VA_ARGS__)
#define __FE_170(__FE, X, ...) __FE(X)__FE_169(__FE, __VA_ARGS__)
#define __FE_171(__FE, X, ...) __FE(X)__FE_170(__FE, __VA_ARGS__)
#define __FE_172(__FE, X, ...) __FE(X)__FE_171(__FE, __VA_ARGS__)
#define __FE_173(__FE, X, ...) __FE(X)__FE_172(__FE, __VA_ARGS__)
#define __FE_174(__FE, X, ...) __FE(X)__FE_173(__FE, __VA_ARGS__)
#define __FE_175(__FE, X, ...) __FE(X)__FE_174(__FE, __VA_ARGS__)
#define __FE_176(__FE, X, ...) __FE(X)__FE_175(__FE, __VA_ARGS__)
#define __FE_177(__FE, X, ...) __FE(X)__FE_176(__FE, __VA_ARGS__)
#define __FE_178(__FE, X, ...) __FE(X)__FE_177(__FE, __VA_ARGS__)
#define __FE_179(__FE, X, ...) __FE(X)__FE_178(__FE, __VA_ARGS__)
#define __FE_180(__FE, X, ...) __FE(X)__FE_179(__FE, __VA_ARGS__)
#define __FE_181(__FE, X, ...) __FE(X)__FE_180(__FE, __VA_ARGS__)
#define __FE_182(__FE, X, ...) __FE(X)__FE_181(__FE, __VA_ARGS__)
#define __FE_183(__FE, X, ...) __FE(X)__FE_182(__FE, __VA_ARGS__)
#define __FE_184(__FE, X, ...) __FE(X)__FE_183(__FE, __VA_ARGS__)
#define __FE_185(__FE, X, ...) __FE(X)__FE_184(__FE, __VA_ARGS__)
#define __FE_186(__FE, X, ...) __FE(X)__FE_185(__FE, __VA_ARGS__)
#define __FE_187(__FE, X, ...) __FE(X)__FE_186(__FE, __VA_ARGS__)
#define __FE_188(__FE, X, ...) __FE(X)__FE_187(__FE, __VA_ARGS__)
#define __FE_189(__FE, X, ...) __FE(X)__FE_188(__FE, __VA_ARGS__)
#define __FE_190(__FE, X, ...) __FE(X)__FE_189(__FE, __VA_ARGS__)
#define __FE_191(__FE, X, ...) __FE(X)__FE_190(__FE, __VA_ARGS__)
#define __FE_192(__FE, X, ...) __FE(X)__FE_191(__FE, __VA_ARGS__)
#define __FE_193(__FE, X, ...) __FE(X)__FE_192(__FE, __VA_ARGS__)
#define __FE_194(__FE, X, ...) __FE(X)__FE_193(__FE, __VA_ARGS__)
#define __FE_195(__FE, X, ...) __FE(X)__FE_194(__FE, __VA_ARGS__)
#define __FE_196(__FE, X, ...) __FE(X)__FE_195(__FE, __VA_ARGS__)
#define __FE_197(__FE, X, ...) __FE(X)__FE_196(__FE, __VA_ARGS__)
#define __FE_198(__FE, X, ...) __FE(X)__FE_197(__FE, __VA_ARGS__)
#define __FE_199(__FE, X, ...) __FE(X)__FE_198(__FE, __VA_ARGS__)
#define __FE_200(__FE, X, ...) __FE(X)__FE_199(__FE, __VA_ARGS__)
#define __FE_201(__FE, X, ...) __FE(X)__FE_200(__FE, __VA_ARGS__)
#define __FE_202(__FE, X, ...) __FE(X)__FE_201(__FE, __VA_ARGS__)
#define __FE_203(__FE, X, ...) __FE(X)__FE_202(__FE, __VA_ARGS__)
#define __FE_204(__FE, X, ...) __FE(X)__FE_203(__FE, __VA_ARGS__)
#define __FE_205(__FE, X, ...) __FE(X)__FE_204(__FE, __VA_ARGS__)
#define __FE_206(__FE, X, ...) __FE(X)__FE_205(__FE, __VA_ARGS__)
#define __FE_207(__FE, X, ...) __FE(X)__FE_206(__FE, __VA_ARGS__)
#define __FE_208(__FE, X, ...) __FE(X)__FE_207(__FE, __VA_ARGS__)
#define __FE_209(__FE, X, ...) __FE(X)__FE_208(__FE, __VA_ARGS__)
#define __FE_210(__FE, X, ...) __FE(X)__FE_209(__FE, __VA_ARGS__)
#define __FE_211(__FE, X, ...) __FE(X)__FE_210(__FE, __VA_ARGS__)
#define __FE_212(__FE, X, ...) __FE(X)__FE_211(__FE, __VA_ARGS__)
#define __FE_213(__FE, X, ...) __FE(X)__FE_212(__FE, __VA_ARGS__)
#define __FE_214(__FE, X, ...) __FE(X)__FE_213(__FE, __VA_ARGS__)
#define __FE_215(__FE, X, ...) __FE(X)__FE_214(__FE, __VA_ARGS__)
#define __FE_216(__FE, X, ...) __FE(X)__FE_215(__FE, __VA_ARGS__)
#define __FE_217(__FE, X, ...) __FE(X)__FE_216(__FE, __VA_ARGS__)
#define __FE_218(__FE, X, ...) __FE(X)__FE_217(__FE, __VA_ARGS__)
#define __FE_219(__FE, X, ...) __FE(X)__FE_218(__FE, __VA_ARGS__)
#define __FE_220(__FE, X, ...) __FE(X)__FE_219(__FE, __VA_ARGS__)
#define __FE_221(__FE, X, ...) __FE(X)__FE_220(__FE, __VA_ARGS__)
#define __FE_222(__FE, X, ...) __FE(X)__FE_221(__FE, __VA_ARGS__)
#define __FE_223(__FE, X, ...) __FE(X)__FE_222(__FE, __VA_ARGS__)
#define __FE_224(__FE, X, ...) __FE(X)__FE_223(__FE, __VA_ARGS__)
#define __FE_225(__FE, X, ...) __FE(X)__FE_224(__FE, __VA_ARGS__)
#define __FE_226(__FE, X, ...) __FE(X)__FE_225(__FE, __VA_ARGS__)
#define __FE_227(__FE, X, ...) __FE(X)__FE_226(__FE, __VA_ARGS__)
#define __FE_228(__FE, X, ...) __FE(X)__FE_227(__FE, __VA_ARGS__)
#define __FE_229(__FE, X, ...) __FE(X)__FE_228(__FE, __VA_ARGS__)
#define __FE_230(__FE, X, ...) __FE(X)__FE_229(__FE, __VA_ARGS__)
#define __FE_231(__FE, X, ...) __FE(X)__FE_230(__FE, __VA_ARGS__)
#define __FE_232(__FE, X, ...) __FE(X)__FE_231(__FE, __VA_ARGS__)
#define __FE_233(__FE, X, ...) __FE(X)__FE_232(__FE, __VA_ARGS__)
#define __FE_234(__FE, X, ...) __FE(X)__FE_233(__FE, __VA_ARGS__)
#define __FE_235(__FE, X, ...) __FE(X)__FE_234(__FE, __VA_ARGS__)
#define __FE_236(__FE, X, ...) __FE(X)__FE_235(__FE, __VA_ARGS__)
#define __FE_237(__FE, X, ...) __FE(X)__FE_236(__FE, __VA_ARGS__)
#define __FE_238(__FE, X, ...) __FE(X)__FE_237(__FE, __VA_ARGS__)
#define __FE_239(__FE, X, ...) __FE(X)__FE_238(__FE, __VA_ARGS__)
#define __FE_240(__FE, X, ...) __FE(X)__FE_239(__FE, __VA_ARGS__)
#define __FE_241(__FE, X, ...) __FE(X)__FE_240(__FE, __VA_ARGS__)
#define __FE_242(__FE, X, ...) __FE(X)__FE_241(__FE, __VA_ARGS__)
#define __FE_243(__FE, X, ...) __FE(X)__FE_242(__FE, __VA_ARGS__)
#define __FE_244(__FE, X, ...) __FE(X)__FE_243(__FE, __VA_ARGS__)
#define __FE_245(__FE, X, ...) __FE(X)__FE_244(__FE, __VA_ARGS__)
#define __FE_246(__FE, X, ...) __FE(X)__FE_245(__FE, __VA_ARGS__)
#define __FE_247(__FE, X, ...) __FE(X)__FE_246(__FE, __VA_ARGS__)
#define __FE_248(__FE, X, ...) __FE(X)__FE_247(__FE, __VA_ARGS__)
#define __FE_249(__FE, X, ...) __FE(X)__FE_248(__FE, __VA_ARGS__)
#define __FE_250(__FE, X, ...) __FE(X)__FE_249(__FE, __VA_ARGS__)
#define __FE_251(__FE, X, ...) __FE(X)__FE_250(__FE, __VA_ARGS__)
#define __FE_252(__FE, X, ...) __FE(X)__FE_251(__FE, __VA_ARGS__)
#define __FE_253(__FE, X, ...) __FE(X)__FE_252(__FE, __VA_ARGS__)
#define __FE_254(__FE, X, ...) __FE(X)__FE_253(__FE, __VA_ARGS__)
#define __FE_255(__FE, X, ...) __FE(X)__FE_254(__FE, __VA_ARGS__)
#define __FE_256(__FE, X, ...) __FE(X)__FE_255(__FE, __VA_ARGS__)

#define __FOR_EACH(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,_128,_129,_130,_131,_132,_133,_134,_135,_136,_137,_138,_139,_140,_141,_142,_143,_144,_145,_146,_147,_148,_149,_150,_151,_152,_153,_154,_155,_156,_157,_158,_159,_160,_161,_162,_163,_164,_165,_166,_167,_168,_169,_170,_171,_172,_173,_174,_175,_176,_177,_178,_179,_180,_181,_182,_183,_184,_185,_186,_187,_188,_189,_190,_191,_192,_193,_194,_195,_196,_197,_198,_199,_200,_201,_202,_203,_204,_205,_206,_207,_208,_209,_210,_211,_212,_213,_214,_215,_216,_217,_218,_219,_220,_221,_222,_223,_224,_225,_226,_227,_228,_229,_230,_231,_232,_233,_234,_235,_236,_237,_238,_239,_240,_241,_242,_243,_244,_245,_246,_247,_248,_249,_250,_251,_252,_253,_254,_255,_256,macroName,...) macroName 
#define FOR_EACH(_MACRO,...) __FOR_EACH(__VA_ARGS__,__FE_256,__FE_255,__FE_254,__FE_253,__FE_252,__FE_251,__FE_250,__FE_249,__FE_248,__FE_247,__FE_246,__FE_245,__FE_244,__FE_243,__FE_242,__FE_241,__FE_240,__FE_239,__FE_238,__FE_237,__FE_236,__FE_235,__FE_234,__FE_233,__FE_232,__FE_231,__FE_230,__FE_229,__FE_228,__FE_227,__FE_226,__FE_225,__FE_224,__FE_223,__FE_222,__FE_221,__FE_220,__FE_219,__FE_218,__FE_217,__FE_216,__FE_215,__FE_214,__FE_213,__FE_212,__FE_211,__FE_210,__FE_209,__FE_208,__FE_207,__FE_206,__FE_205,__FE_204,__FE_203,__FE_202,__FE_201,__FE_200,__FE_199,__FE_198,__FE_197,__FE_196,__FE_195,__FE_194,__FE_193,__FE_192,__FE_191,__FE_190,__FE_189,__FE_188,__FE_187,__FE_186,__FE_185,__FE_184,__FE_183,__FE_182,__FE_181,__FE_180,__FE_179,__FE_178,__FE_177,__FE_176,__FE_175,__FE_174,__FE_173,__FE_172,__FE_171,__FE_170,__FE_169,__FE_168,__FE_167,__FE_166,__FE_165,__FE_164,__FE_163,__FE_162,__FE_161,__FE_160,__FE_159,__FE_158,__FE_157,__FE_156,__FE_155,__FE_154,__FE_153,__FE_152,__FE_151,__FE_150,__FE_149,__FE_148,__FE_147,__FE_146,__FE_145,__FE_144,__FE_143,__FE_142,__FE_141,__FE_140,__FE_139,__FE_138,__FE_137,__FE_136,__FE_135,__FE_134,__FE_133,__FE_132,__FE_131,__FE_130,__FE_129,__FE_128,__FE_127,__FE_126,__FE_125,__FE_124,__FE_123,__FE_122,__FE_121,__FE_120,__FE_119,__FE_118,__FE_117,__FE_116,__FE_115,__FE_114,__FE_113,__FE_112,__FE_111,__FE_110,__FE_109,__FE_108,__FE_107,__FE_106,__FE_105,__FE_104,__FE_103,__FE_102,__FE_101,__FE_100,__FE_99,__FE_98,__FE_97,__FE_96,__FE_95,__FE_94,__FE_93,__FE_92,__FE_91,__FE_90,__FE_89,__FE_88,__FE_87,__FE_86,__FE_85,__FE_84,__FE_83,__FE_82,__FE_81,__FE_80,__FE_79,__FE_78,__FE_77,__FE_76,__FE_75,__FE_74,__FE_73,__FE_72,__FE_71,__FE_70,__FE_69,__FE_68,__FE_67,__FE_66,__FE_65,__FE_64,__FE_63,__FE_62,__FE_61,__FE_60,__FE_59,__FE_58,__FE_57,__FE_56,__FE_55,__FE_54,__FE_53,__FE_52,__FE_51,__FE_50,__FE_49,__FE_48,__FE_47,__FE_46,__FE_45,__FE_44,__FE_43,__FE_42,__FE_41,__FE_40,__FE_39,__FE_38,__FE_37,__FE_36,__FE_35,__FE_34,__FE_33,__FE_32,__FE_31,__FE_30,__FE_29,__FE_28,__FE_27,__FE_26,__FE_25,__FE_24,__FE_23,__FE_22,__FE_21,__FE_20,__FE_19,__FE_18,__FE_17,__FE_16,__FE_15,__FE_14,__FE_13,__FE_12,__FE_11,__FE_10,__FE_9,__FE_8,__FE_7,__FE_6,__FE_5,__FE_4,__FE_3,__FE_2,__FE_1)(_MACRO,__VA_ARGS__)


//////////////////////////////////////////////////////////
// CPU Affinity

#ifdef _WINDOWS
	#define ___CPU_AFFINITY_ADD___(n) processAffinityMask |= (1 << ((n) % std::thread::hardware_concurrency()));
	#define SET_CPU_AFFINITY(...) {\
		DWORD_PTR processAffinityMask = 0;\
		FOR_EACH(___CPU_AFFINITY_ADD___, __VA_ARGS__)\
		if (!SetThreadAffinityMask(GetCurrentThread(), processAffinityMask)) LOG_ERROR("Error calling SetThreadAffinityMask");\
	}
	#define ___CPU_AFFINITY_REMOVE___(n) processAffinityMask &= ~(1 << ((n) % std::thread::hardware_concurrency()));
	#define UNSET_CPU_AFFINITY(...) {\
		DWORD_PTR processAffinityMask = 0;\
		for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) ___CPU_AFFINITY_ADD___(i)\
		FOR_EACH(___CPU_AFFINITY_REMOVE___, __VA_ARGS__)\
		if (!SetThreadAffinityMask(GetCurrentThread(), processAffinityMask)) LOG_ERROR("Error calling SetThreadAffinityMask");\
	}
	#define SET_THREAD_HIGHEST_PRIORITY() {\
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);\
	}
#else
	#define ___CPU_AFFINITY_ADD___(n) CPU_SET((n) % std::thread::hardware_concurrency(), &cpuset);
	#define SET_CPU_AFFINITY(...) {\
		cpu_set_t cpuset;\
		CPU_ZERO(&cpuset);\
		FOR_EACH(___CPU_AFFINITY_ADD___, __VA_ARGS__)\
		int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);\
		if (rc != 0) LOG_ERROR("Error calling pthread_setaffinity_np: error " << std::strerror(rc))\
	}
	#define ___CPU_AFFINITY_REMOVE___(n) CPU_CLR((n) % std::thread::hardware_concurrency(), &cpuset);
	#define UNSET_CPU_AFFINITY(...) {\
		cpu_set_t cpuset;\
		for (int i = 0; i < std::thread::hardware_concurrency(); ++i) ___CPU_AFFINITY_ADD___(i)\
		FOR_EACH(___CPU_AFFINITY_REMOVE___, __VA_ARGS__)\
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
