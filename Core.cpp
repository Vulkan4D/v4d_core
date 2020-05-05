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

	void V4D_SIGNAL_HANDLER(int num) {
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
