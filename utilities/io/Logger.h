#pragma once
#include <v4d.h>

namespace v4d::io {
	// https://misc.flogisoft.com/bash/tip_colors_and_formatting

	class V4DLIB Logger {
	private:
		std::string filepath;
		std::atomic<bool> useLogFile;
		std::atomic<bool> verbose;

		std::mutex mu;
		std::once_flag readFileOnce, setVerboseOnce;
		std::ofstream file;

		void LogToFile(const std::string& message);

	public:

		Logger();
		Logger(const std::string& filepath, std::optional<bool> verbose = std::nullopt);

		~Logger();

		DELETE_COPY_MOVE_CONSTRUCTORS(Logger)

		static std::shared_ptr<Logger> ConsoleInstance(std::optional<bool> verbose = std::nullopt);
		
		static std::shared_ptr<Logger> FileInstance(const std::string& filepath, std::optional<bool> verbose = std::nullopt);

		void Log(std::ostream& message, const char* style = "0");

		std::string GetCurrentThreadIdStr() const;

		// template<typename T>
		// inline void Log(T&& message, const char* style = "0") {
		// 	Log(std::ostringstream() << message, style);
		// }
		
		inline void LogError(std::ostream& message) {
			Log(message, "1;31");
		}

		inline bool IsVerbose() const {
			return verbose;
		}

		inline void SetVerbose(bool isVerbose = true) {
			verbose = isVerbose;
		}

	};
}


/////////////////////////////////////////////////////////////////////////
// stream casts
V4DLIB std::ostream& operator<<(std::ostream& stream, std::vector<byte> bytes);


/////////////////////////////////////////////////////////////////////////
// MACROS

#ifdef _V4D_CORE
	#define LOGGER_INSTANCE v4d::Core::coreLogger
	#ifndef LOGGER_PREFIX
		#define LOGGER_PREFIX " [V4D Core] "
	#endif
#else
	#ifndef LOGGER_INSTANCE
		#define LOGGER_INSTANCE v4d::io::Logger::ConsoleInstance()
	#endif
#endif

#ifndef LOGGER_PREFIX
	#define LOGGER_PREFIX ""
#endif

// Logging into console only in debug mode
#ifdef _DEBUG
	#define __LOG_PREPEND_THREAD_ID__ << (LOGGER_INSTANCE->IsVerbose()? LOGGER_INSTANCE->GetCurrentThreadIdStr() : std::string(""))
	#define __LOG_APPEND_FILE_AND_LINE__ << " [" << __FILE__ << ":" << __LINE__ << "]"
	#define DEBUG(msg) LOGGER_INSTANCE->Log(std::ostringstream() << LOGGER_PREFIX << "DEBUG: " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__, "1;33");
	#define DEBUG_WARN(msg) LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "WARNING(debug): " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
	#define DEBUG_ERROR(msg) LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "ERROR(debug): " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
	#define INVALIDCODE(msg) LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "INVALIDCODE(debug): " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
#else
	#define __LOG_PREPEND_THREAD_ID__
	#define __LOG_APPEND_FILE_AND_LINE__
	#define DEBUG(msg)
	#define DEBUG_WARN(msg)
	#define DEBUG_ERROR(msg)
	#define INVALIDCODE(msg)
#endif
#define LOG_DEBUG(msg) DEBUG(msg)

// Info in console
#define LOG(msg) LOGGER_INSTANCE->Log(std::ostringstream() << LOGGER_PREFIX __LOG_PREPEND_THREAD_ID__ << msg, LOGGER_INSTANCE->IsVerbose()? "1":"0");

// Info in console (shown only in verbose mode)
#define LOG_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOGGER_INSTANCE->Log(std::ostringstream() << LOGGER_PREFIX __LOG_PREPEND_THREAD_ID__ << msg, "2");}

// Success/Warning/Error messages in console
#define LOG_SUCCESS(msg) LOGGER_INSTANCE->Log(std::ostringstream() << LOGGER_PREFIX __LOG_PREPEND_THREAD_ID__ << msg, "1;36");
#define LOG_WARN(msg) LOGGER_INSTANCE->Log(std::ostringstream() << LOGGER_PREFIX << "WARNING: " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__, "1;33");
#define LOG_ERROR(msg) LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "ERROR: " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
// Verbose
#define LOG_SUCCESS_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOG_SUCCESS(msg)};
#define LOG_WARN_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOG_WARN(msg)};
#define LOG_ERROR_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOG_ERROR(msg)};


// Fatal errors that should Log the event and terminate the application

// error message in console and exit(1)
// #define FATAL_EXIT(msg) {LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "FATAL(exit): " << msg __LOG_APPEND_FILE_AND_LINE__); std::exit(1);}

// error message in console and abort() (causes breakpoint in debug)
// #define FATAL_ABORT(msg) {LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "FATAL(abort): " << msg __LOG_APPEND_FILE_AND_LINE__); std::abort();}

// error message in console and raise SIGINT (causes breakpoint in debug)
#define FATAL(msg) {LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "FATAL(interupt): " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__); raise(SIGINT);}

// error message in console and raise SIGKILL (emergency kill the application)
#define FATAL_KILL(msg) {LOGGER_INSTANCE->LogError(std::ostringstream() << LOGGER_PREFIX << "FATAL(kill): " __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__); raise(SIGKILL);}

