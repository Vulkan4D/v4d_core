#include "Logger.h"
#include <thread>
#include <cstring>

using namespace v4d::io;

Logger::Logger() : filepath(""), useLogFile(false), verbose(false) {}
Logger::Logger(const std::string& filepath, std::optional<bool> verbose) : filepath(filepath), useLogFile(filepath != ""), verbose(verbose.has_value() && verbose.value()) {}

Logger::~Logger() {
	file.close();
}

std::shared_ptr<Logger> Logger::ConsoleInstance(std::optional<bool> verbose) {
	static std::shared_ptr<Logger> consoleLogger = std::make_shared<Logger>();
	if (verbose.has_value()) consoleLogger->SetVerbose(verbose.value());
	return consoleLogger;
}

std::shared_ptr<Logger> Logger::FileInstance(const std::string& filepath, std::optional<bool> verbose) 
	STATIC_CLASS_INSTANCES_CPP((std::string)filepath, Logger, filepath, verbose)

void Logger::LogToFile(const std::string& message) {
	std::call_once(readFileOnce, [&f=file, &filepath=filepath](){
		f.open(filepath);
	});
	file << message << std::endl;
}

void Logger::Log(const std::ostream& message, const char* style) {
	std::lock_guard lock(mu);
	std::string msg = dynamic_cast<const std::ostringstream&>(message).str();
	try {
		if (useLogFile) {
			LogToFile(msg);
		} else {
			#if defined(_WINDOWS) || defined(V4D_LOGGER_DONT_STYLE)
				if (style[0] == '1')
					std::cerr << msg << std::endl;
				else
					std::cout << msg << std::endl;
			#else
				if (style[0] == '1')
					std::cerr << "\033[" << style << 'm' << msg << "\033[0m" << std::endl;
				else
					std::cout << "\033[" << style << 'm' << msg << "\033[0m" << std::endl;
			#endif
		}
	} catch(...) {}
}

std::string Logger::GetCurrentThreadIdStr() const {
	std::stringstream str("");
	str << " [thread " << std::this_thread::get_id() << "] ";
	return str.str();
}

/////////////////////////////////////////////////////////////////////////
// stream casts
std::ostream& operator<<(std::ostream& stream, const std::vector<byte>& bytes) {
	stream << "bytes[";
	bool first = true;
	for (const auto& b : bytes) {
		if (first) {
			first = false;
		} else {
			stream << ",";
		}
		stream << (int)b;
	}
	return stream << "]";
}
