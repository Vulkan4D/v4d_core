#include <v4d.h>

#ifdef _V4D_CORE

	#if defined(XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE) && defined(XVK_EXPORT)
		#include <xvkInterface.c>
	#endif
	
	//////////////////////////////////////////////////////////
	// V4D global events

	// Signals
	DEFINE_CORE_EVENT_BODY(SIGNAL, int)
	DEFINE_CORE_EVENT_BODY(SIGNAL_TERMINATE, int)
	DEFINE_CORE_EVENT_BODY(SIGNAL_INTERUPT, int)
	DEFINE_CORE_EVENT_BODY(SIGNAL_HANGUP, int)
	DEFINE_CORE_EVENT_BODY(SIGNAL_ABORT, int)
	DEFINE_CORE_EVENT_BODY(SIGNAL_QUIT, int)
	DEFINE_CORE_EVENT_BODY(APP_KILLED, int)
	DEFINE_CORE_EVENT_BODY(APP_ERROR, int)

	const char* SIGNALS_STR[32] {
		/*0*/ ""
		#ifdef _WINDOWS
			,/*1*/ "SIGHUP"
			,/*2*/ "SIGINT"
			,/*3*/ "SIGQUIT"
			,/*4*/ "SIGILL"
			,/*5*/ "SIGTRAP"
			,/*6*/ "SIGIOT or SIGABRT_COMPAT"
			,/*7*/ "SIGEMT"
			,/*8*/ "SIGFPE"
			,/*9*/ "SIGKILL"
			,/*10*/ "SIGBUS"	
			,/*11*/ "SIGSEGV"
			,/*12*/ "SIGSYS"
			,/*13*/ "SIGPIPE"
			,/*14*/ "SIGALRM"
			,/*15*/ "SIGTERM"
			,/*16*/ "16"
			,/*17*/ "17"
			,/*18*/ "18"
			,/*19*/ "19"
			,/*20*/ "20"
			,/*21*/ "SIGBREAK"
			,/*22*/ "SIGABRT"
			,/*23*/ "NSIG"
		#else
			,/*1*/"SIGHUP"		/* Hangup */
			,/*2*/"SIGINT"		/* Interactive attention signal */
			,/*3*/"SIGQUIT"		/* Quit */
			,/*4*/"SIGILL"		/* Illegal instruction */
			,/*5*/"SIGTRAP"		/* Trace/breakpoint trap */
			,/*6*/"SIGABRT"		/* Abnormal termination */
			,/*7*/"SIGBUS"		/* Bus error */
			,/*8*/"SIGFPE"		/* Erroneous arithmetic operation */
			,/*9*/"SIGKILL"		/* Killed */
			,/*10*/"SIGUSR1"	/* User-defined signal 1 */
			,/*11*/"SIGSEGV"	/* Invalid access to storage */
			,/*12*/"SIGUSR2"	/* User-defined signal 2 */
			,/*13*/"SIGPIPE"	/* Broken pipe */
			,/*14*/"SIGALRM"	/* Alarm clock */
			,/*15*/"SIGTERM"	/* Termination request */
			,/*16*/"SIGSTKFLT"	/* Stack fault (obsolete) */
			,/*17*/"SIGCHLD"	/* Child terminated or stopped */
			,/*18*/"SIGCONT"	/* Continue */
			,/*19*/"SIGSTOP"	/* Stop, unblockable */
			,/*20*/"SIGTSTP"	/* Keyboard stop */
			,/*21*/"SIGTTIN"	/* Background read from control terminal */
			,/*22*/"SIGTTOU"	/* Background write to control terminal */
			,/*23*/"SIGURG"		/* Urgent data is available at a socket */
		#endif
			,/*24*/"SIGXCPU"		/* CPU time limit exceeded */
			,/*25*/"SIGXFSZ"		/* File size limit exceeded */
			,/*26*/"SIGVTALRM"		/* Virtual timer expired */
			,/*27*/"SIGPROF"		/* Profiling timer expired */
			,/*28*/"SIGWINCH"		/* Window size change (4.3 BSD, Sun) */
			,/*29*/"SIGPOLL or SIGIO"/* Pollable event occurred (System V) or I/O now possible (4.2 BSD) */
			,/*30*/"SIGPWR"			/* Power failure imminent */
			,/*31*/"SIGSYS"			/* Bad system call */
	};

	void V4D_SIGNAL_HANDLER(int num) {
		try {
			v4d::event::SIGNAL(num);
			switch (num) {
				// Signals that kill the application
				case SIGINT:
					v4d::event::SIGNAL_INTERUPT(num);
					v4d::event::APP_KILLED(num);
					exit(num);
				break;
				case SIGABRT:
					v4d::event::SIGNAL_ABORT(num);
					v4d::event::APP_KILLED(num);
					exit(num);
				break;
				case SIGTERM:
					v4d::event::SIGNAL_TERMINATE(num);
					v4d::event::APP_KILLED(num);
					exit(num);
				break;
				#ifdef _LINUX
					case SIGHUP:
						v4d::event::SIGNAL_HANGUP(num);
						v4d::event::APP_KILLED(num);
						exit(num);
					break;
					case SIGQUIT:
						v4d::event::SIGNAL_QUIT(num);
						v4d::event::APP_KILLED(num);
						exit(num);
					break;
				#endif
				// Errors
				case SIGFPE:
				case SIGILL:
				case SIGSEGV:
					v4d::event::APP_ERROR(num);
				break;
			}
		} catch(...) {}
	}


	//////////////////////////////////////////////////////////
	// V4D global functions

	const std::string v4d::GetCoreBuildVersion() noexcept {
		return V4D_VERSION;
	}

	//////////////////////////////////////////////////////////
	// V4D Core Instance

	std::shared_ptr<v4d::io::Logger> v4d::Core::coreLogger = v4d::io::Logger::ConsoleInstance();

#endif
