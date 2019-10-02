#include <v4d.h>

using namespace v4d::io;

std::shared_ptr<Logger> Logger::ConsoleInstance(std::optional<bool> verbose) {
	static std::shared_ptr<Logger> consoleLogger = std::make_shared<Logger>();
	if (verbose.has_value()) consoleLogger->SetVerbose(verbose.value());
	return consoleLogger;
}

std::shared_ptr<Logger> Logger::FileInstance(const std::string& filepath, std::optional<bool> verbose) {
	static std::map<std::string, std::shared_ptr<Logger>> fileInstances {};
	if (fileInstances.find(filepath) == fileInstances.end()) {
		fileInstances.emplace(filepath, std::make_shared<Logger>(filepath, verbose.has_value() && verbose.value()));
	}
	return fileInstances[filepath];
}


/////////////////////////////////////////////////////////////////////////
// stream casts
std::ostream& operator<<(std::ostream& stream, std::vector<byte> bytes) {
	stream << "bytes[";
	bool first = true;
	for (auto b : bytes) {
		if (first) {
			first = false;
		} else {
			stream << ",";
		}
		stream << (int)b;
	}
	return stream << "]";
}
