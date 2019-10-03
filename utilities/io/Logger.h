#pragma once

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

		inline void LogToFile(const std::string& message) {
			std::call_once(readFileOnce, [&f=file, &filepath=filepath, &message](){
				f.open(filepath);
			});
			file << message << std::endl;
		}

	public:

		Logger() : filepath(""), useLogFile(false), verbose(false) {}
		Logger(const std::string& filepath, bool verbose = false) : filepath(filepath), useLogFile(filepath != ""), verbose(verbose) {}

		~Logger() {
			file.close();
		}

		DELETE_COPY_MOVE_CONSTRUCTORS(Logger)

		static std::shared_ptr<Logger> ConsoleInstance(std::optional<bool> verbose = std::nullopt);
		
		static std::shared_ptr<Logger> FileInstance(const std::string& filepath, std::optional<bool> verbose = std::nullopt);

		INLINE void Log(std::ostream& message, const char* style = "0") {
			std::lock_guard lock(mu);
			std::string msg = dynamic_cast<std::ostringstream&>(message).str();
			try {
				if (useLogFile) {
					LogToFile(msg);
				} else {
					#ifdef _WINDOWS
						if (strcmp(style, "1"))
							std::cerr << msg << std::endl;
						else
							std::cout << msg << std::endl;
					#else
						if (strcmp(style, "1"))
							std::cerr << "\033[" << style << 'm' << msg << "\033[0m" << std::endl;
						else
							std::cout << "\033[" << style << 'm' << msg << "\033[0m" << std::endl;
					#endif
				}
			} catch(...) {}
		}

		INLINE std::string GetCurrentThreadIdStr() const {
			std::stringstream str("");
			str << " [thread " << std::this_thread::get_id() << "] ";
			return str.str();
		}

		template<typename T>
		INLINE void Log(T&& message, const char* style = "0") {
			Log(std::ostringstream().flush() << message, style);
		}
		
		INLINE void LogError(std::ostream& message) {
			Log(message, "1;31");
		}

		INLINE bool IsVerbose() const {
			return verbose;
		}

		INLINE void SetVerbose(bool isVerbose = true) {
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
#else
	#ifndef LOGGER_INSTANCE
		#define LOGGER_INSTANCE v4d::io::Logger::ConsoleInstance()
	#endif
#endif

// Logging into console only in debug mode
#ifdef _DEBUG
	#define __LOG_PREPEND_THREAD_ID__ << (LOGGER_INSTANCE->IsVerbose()? LOGGER_INSTANCE->GetCurrentThreadIdStr() : std::string(""))
	#define __LOG_APPEND_FILE_AND_LINE__ << " [" << __FILE__ << ":" << __LINE__ << "]"
	#define DEBUG(msg) LOGGER_INSTANCE->Log(std::ostringstream("DEBUG: ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__, "1;33");
	#define DEBUG_WARN(msg) LOGGER_INSTANCE->LogError(std::ostringstream("WARNING(debug): ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
	#define DEBUG_ERROR(msg) LOGGER_INSTANCE->LogError(std::ostringstream("ERROR(debug): ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
	#define INVALIDCODE(msg) LOGGER_INSTANCE->LogError(std::ostringstream("INVALIDCODE(debug): ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
#else
	#define __LOG_PREPEND_THREAD_ID__
	#define __LOG_APPEND_FILE_AND_LINE__
	#define DEBUG(msg)
	#define DEBUG_WARN(msg)
	#define DEBUG_ERROR(msg)
	#define INVALIDCODE(msg)
#endif

// Info in console
#define LOG(msg) LOGGER_INSTANCE->Log(std::ostringstream("").flush() __LOG_PREPEND_THREAD_ID__ << msg, LOGGER_INSTANCE->IsVerbose()? "1":"0");

// Info in console (shown only in verbose mode)
#define LOG_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOGGER_INSTANCE->Log(std::ostringstream("").flush() __LOG_PREPEND_THREAD_ID__ << msg, "2");}

// Success/Warning/Error messages in console
#define LOG_SUCCESS(msg) LOGGER_INSTANCE->Log(std::ostringstream("").flush() __LOG_PREPEND_THREAD_ID__ << msg, "1;36");
#define LOG_WARN(msg) LOGGER_INSTANCE->Log(std::ostringstream("WARNING: ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__, "1;33");
#define LOG_ERROR(msg) LOGGER_INSTANCE->LogError(std::ostringstream("ERROR: ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__);
// Verbose
#define LOG_SUCCESS_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOG_SUCCESS(msg)};
#define LOG_WARN_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOG_WARN(msg)};
#define LOG_ERROR_VERBOSE(msg) {if (LOGGER_INSTANCE->IsVerbose()) LOG_ERROR(msg)};


// Fatal errors that should Log the event and terminate the application

// error message in console and exit(1)
// #define FATAL_EXIT(msg) {LOGGER_INSTANCE->LogError(ostringstream("FATAL(exit): ").flush() << msg __LOG_APPEND_FILE_AND_LINE__); std::exit(1);}

// error message in console and abort() (causes breakpoint in debug)
// #define FATAL_ABORT(msg) {LOGGER_INSTANCE->LogError(ostringstream("FATAL(abort): ").flush() << msg __LOG_APPEND_FILE_AND_LINE__); std::abort();}

// error message in console and raise SIGINT (causes breakpoint in debug)
#define FATAL(msg) {LOGGER_INSTANCE->LogError(ostringstream("FATAL(interupt): ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__); raise(SIGINT);}

// error message in console and raise SIGKILL (emergency kill the application)
#define FATAL_KILL(msg) {LOGGER_INSTANCE->LogError(ostringstream("FATAL(kill): ").flush() __LOG_PREPEND_THREAD_ID__ << msg __LOG_APPEND_FILE_AND_LINE__); raise(SIGKILL);}

