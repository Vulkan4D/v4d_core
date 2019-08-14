// Library import/export
#if defined _WINDOWS
	#ifdef _V4D_CORE
    	#define V4DLIB __declspec(dllexport)
	#else// Project/System
		#define V4DLIB __declspec(dllimport)
	#endif
	#ifdef _V4D_SYSTEM
    	#define V4DSYSTEM extern "C" __declspec(dllexport)
	#else// Project/Core
		#define V4DSYSTEM __declspec(dllimport)
	#endif
#else
	#define V4DLIB
	#ifdef _V4D_SYSTEM
    	#define V4DSYSTEM extern "C" 
	#else// Project/Core
		#define V4DSYSTEM
	#endif
#endif

// Pause current thread for specified duration (ex: 5s = 5 seconds)
#define SLEEP(x) {using namespace std::literals::chrono_literals; std::this_thread::sleep_for(x);}
