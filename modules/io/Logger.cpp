#include <v4d.h>

using namespace v4d::io;

std::shared_ptr<Logger> Logger::ConsoleInstance(const bool& verbose) {
	static std::shared_ptr<Logger> consoleLogger = std::make_shared<Logger>();
	consoleLogger->SetVerbose(verbose);
	return consoleLogger;
}

std::shared_ptr<Logger> Logger::FileInstance(const std::string& filepath, const bool& verbose) {
	static std::map<std::string, std::shared_ptr<Logger>> fileInstances {};
	if (fileInstances.find(filepath) == fileInstances.end()) {
		fileInstances.emplace(filepath, std::make_shared<Logger>(filepath, verbose));
	}
	return fileInstances[filepath];
}

