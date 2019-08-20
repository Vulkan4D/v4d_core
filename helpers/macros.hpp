// Pause current thread for specified duration (ex: 5s = 5 seconds)
#define SLEEP(x) {using namespace std::literals::chrono_literals; std::this_thread::sleep_for(x);}

// Force Inline
#ifdef _MSC_VER
	#define INLINE __forceinline
#else
	#define INLINE __attribute__((always_inline))
#endif

