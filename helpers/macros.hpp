// Library import/export
#if defined _WINDOWS
	#ifdef _V4D_CORE
    	#define V4DLIB __declspec(dllexport)
	#else
		#define V4DLIB __declspec(dllimport)
	#endif
#else
	#define V4DLIB
#endif

// Pause current thread for specified duration (ex: 5s = 5 seconds)
#define SLEEP(x) {using namespace std::literals::chrono_literals; std::this_thread::sleep_for(x);}

