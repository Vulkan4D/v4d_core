/**
 * This helper is part of Vulkan4D (open source project)
 * 
 * @author Olivier St-Laurent
 * @date 2019-06-18
 */
#pragma once

// Now included in common.hh
	// #include <iostream>
	// #include <atomic>
	// #include <mutex>
	// #include <fstream>
	// #include <string>
	// #include <sstream>
	// #include <map>

// Info in console
#define LOG(x) v4d::Logger::ConsoleInstance().Log(std::ostringstream().flush() << x, v4d::Logger::ConsoleInstance().IsVerbose()? "1":"0");

// Info in console (shown only in verbose mode)
#define LOG_VERBOSE(x) {if (v4d::Logger::ConsoleInstance().IsVerbose()) v4d::Logger::ConsoleInstance().Log(std::ostringstream().flush() << x, "2");}

// Success/Warning/Error messages in console
#define LOG_SUCCESS(x) v4d::Logger::ConsoleInstance().Log(std::ostringstream().flush() << x, "1;36");
#define LOG_WARN(x) v4d::Logger::ConsoleInstance().Log(std::ostringstream("WARNING: ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]", "1;33");
#define LOG_ERROR(x) v4d::Logger::ConsoleInstance().LogError(std::ostringstream("ERROR: ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]");

// Logging into console only in debug mode
#ifdef _DEBUG
	#define DEBUG(x) v4d::Logger::ConsoleInstance().Log(std::ostringstream("DEBUG: ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]", "1;33");
	#define DEBUG_WARN(x) v4d::Logger::ConsoleInstance().LogError(std::ostringstream("WARNING(debug): ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]");
	#define DEBUG_ERROR(x) v4d::Logger::ConsoleInstance().LogError(std::ostringstream("ERROR(debug): ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]");
	#define INVALIDCODE(x) v4d::Logger::ConsoleInstance().LogError(std::ostringstream("INVALIDCODE(debug): ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]");
#else
	#define DEBUG(x)
	#define DEBUG_WARN(x)
	#define DEBUG_ERROR(x)
	#define INVALIDCODE(x)
#endif

namespace v4d {
	// https://misc.flogisoft.com/bash/tip_colors_and_formatting

	class V4DLIB Logger {
	private:
		std::string filepath;
		std::atomic<bool> useLogFile;
		bool verbose;

		std::mutex mu;
		std::once_flag once;
		std::ofstream file;

		inline void LogToFile(const std::string &message) {
			std::call_once(once, [&f=file, &filepath=filepath, &message](){
				f.open(filepath);
			});
			file << message << endl;
		}

	public:
		Logger() : filepath(""), useLogFile(false), verbose(false) {}

		~Logger() {
			file.close();
		}

		static Logger& ConsoleInstance(const bool verbose = false) {
			static Logger consoleLogger;
			consoleLogger.SetVerbose(verbose);
			return consoleLogger;
		}
		
		static Logger& FileInstance(const std::string filepath, const bool verbose = false) {
			static std::map<std::string, Logger> fileInstances;
			fileInstances[filepath].Init(filepath, verbose);
			return fileInstances[filepath];
		}

		inline void Log(std::ostream &message, const char* style = "0") {
			string msg = dynamic_cast<std::ostringstream&>(message).str();
			mu.lock();
			try {
				if (useLogFile) {
					LogToFile(msg);
				} else {
					#ifdef _WINDOWS
						std::cout << msg << std::endl;
					#else
						std::cout << "\033[" << style << 'm' << msg << "\033[0m" << std::endl;
					#endif
				}
			} catch(...) {}
			mu.unlock();
		}

		inline void LogError(std::ostream& message) {
			Log(message, "1;31");
		}

		inline bool IsVerbose() {
			return verbose;
		}

		inline void Init(std::string filepath = "", bool verbose = false) {
			this->filepath = filepath;
			this->useLogFile = (filepath != "");
			this->verbose = verbose;
		}

		inline void SetVerbose(bool isVerbose = true) {
			verbose = isVerbose;
		}

		Logger(Logger const&) = delete;
		void operator=(Logger const&) = delete;
	};


}
