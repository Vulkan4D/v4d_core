#include <v4d.h>

using namespace v4d::io;


int ConfigFile::globalAutoReloadInterval = DEFAULT_AUTORELOAD_INTERVAL;

ConfigFile::ConfigFile(const std::string& filePath, std::optional<int> autoReloadInterval) : FilePath(filePath), autoReloadInterval(autoReloadInterval.value_or(globalAutoReloadInterval)) {
	AutoCreateFile();
	if (autoReloadInterval > 0) {
		StartAutoReloadThread();
	}
}

ConfigFile::~ConfigFile() {
	autoReloadInterval = 0;
	{
		std::lock_guard lock(mu);
	}
	if (autoReloadThread && autoReloadThread->joinable()) {
		autoReloadThread->join();
		delete autoReloadThread;
	}
}

void ConfigFile::Load() {
	std::lock_guard lock(mu);
	lastWriteTimeCache = GetLastWriteTime();
	if (lastWriteTimeCache == 0) {
		AutoCreateFile();
		lastWriteTimeCache = GetLastWriteTime();
	}
	ReadConfig();
	lastWriteTimeCache = GetLastWriteTime();
}

void ConfigFile::SetAutoReloadInterval(int interval) {
	std::lock_guard lock(mu);
	autoReloadInterval = interval;
	if (autoReloadInterval <= 0 && autoReloadThread && autoReloadThread->joinable()) {
		autoReloadThread->detach();
		delete autoReloadThread;
		autoReloadThread = nullptr;
	} else if (autoReloadInterval > 0 && !autoReloadThread) {
		StartAutoReloadThread();
	}
}

bool ConfigFile::FileHasChanged() const {
	std::lock_guard lock(mu);
	return lastWriteTimeCache != GetLastWriteTime();
}

void ConfigFile::StartAutoReloadThread() {
	autoReloadThread = new std::thread([this]{
		int interval;
		{
			std::lock_guard lock(mu);
			interval = autoReloadInterval;
		}
		for (;;) {
			if (interval <= 0) return;
			SLEEP(std::chrono::milliseconds{interval})
			std::lock_guard lock(mu);
			if (autoReloadInterval > 0) {
				auto lastWriteTime = GetLastWriteTime();
				if (lastWriteTime == 0) {
					AutoCreateFile();
					lastWriteTime = GetLastWriteTime();
				}
				if (lastWriteTimeCache != lastWriteTime) {
					ReadConfig();
					lastWriteTimeCache = GetLastWriteTime();
				}
			}
			interval = autoReloadInterval;
		}
	});
}

void ConfigFile::ReadFromINI(const std::string& section, std::vector<Conf> configs, bool writeIfNotExists) {
	LOG_VERBOSE("Config File Read: " << filePath.string())
	std::ifstream file(filePath);
	std::string curSection = "";
	std::string line;
	std::cmatch match;
	int n = 0;
	while (!configs.empty() && std::getline(file, line)) {
		n++;
		v4d::String::Trim(line);
		if (line.length() == 0) continue;
		if (std::regex_match(line.c_str(), match, INI_REGEX_COMMENT)) continue;
		if (std::regex_match(line.c_str(), match, INI_REGEX_SECTION)) {
			curSection = match[1];
			continue;
		}
		if (std::regex_match(line.c_str(), match, INI_REGEX_CONF)) {
			for (auto it = configs.begin(); it != configs.end(); it++) {
				auto conf = *it;
				if (conf.name == match[1]) {
					conf.ReadValue(match[2]);
					configs.erase(it);
					break;
				}
			}
			continue;
		}
		LOG_WARN_VERBOSE("Line " << n << " is invalid in config file " << filePath)
	}
	file.close();
	if (writeIfNotExists && !configs.empty()) {
		WriteToINI(section, configs);
	}
}

void ConfigFile::WriteToINI(const std::string& section, std::vector<Conf> configs) {
	LOG_VERBOSE("Config File Write: " << filePath.string())

	// Read all lines
	std::vector<std::string> lines;
	{
		std::ifstream file(filePath);
		std::string line;
		while (std::getline(file, line)) {
			v4d::String::Trim(line);
			lines.push_back(line);
		}
		file.close();
	}

	// Add/Set configurations to lines (at the correct positions)
	for (auto conf : configs) {
		std::string confLine = conf.name + " = " + conf.WriteValue();
		std::string curSection = "";
		std::cmatch match;
		for (auto line = lines.begin(); line != lines.end(); line++) {
			if (section == curSection && std::regex_match(line->c_str(), match, INI_REGEX_SECTION)) {
				// We're at the end of the correct section, Insert conf just before the current line (the next section line)
				auto insertedLine = lines.insert(line, confLine);
				lines.insert(insertedLine+1, ""); // Add an empty line before the next section
				goto NextConf;
			} else {
				if (line->length() == 0) continue;
				if (std::regex_match(line->c_str(), match, INI_REGEX_COMMENT)) continue;
				if (std::regex_match(line->c_str(), match, INI_REGEX_SECTION)) {
					curSection = match[1];
					continue;
				}
			}
			if (std::regex_match(line->c_str(), match, INI_REGEX_CONF)) {
				if (conf.name == match[1]) {
					// Edit existing line in file
					*line = confLine;
					goto NextConf;
				}
				continue;
			}
		}
		// Conf not found in file, append it
		if (section != curSection) {
			// Section not found, append it
			lines.push_back("");
			lines.push_back(std::string("[") + section + "]");
		}
		lines.push_back(confLine);
		NextConf: continue;
	}

	// Write all lines to file
	std::ofstream file(filePath);
	for (auto line : lines) {
		file << line << "\n";
	}
	file.close();
}

void ConfigFile::ReadFromINI(std::function<void(const std::string& section, std::vector<ConfLineStream>& configs)>&& callbackPerSection) {
	LOG_VERBOSE("Config File Read: " << filePath.string())
	std::ifstream file(filePath);
	std::string curSection = "";
	std::vector<ConfLineStream> curConfigs {};
	std::string line;
	std::cmatch match;
	int n = 0;
	while (std::getline(file, line)) {
		n++;
		v4d::String::Trim(line);
		if (line.length() == 0) continue;
		if (std::regex_match(line.c_str(), match, INI_REGEX_COMMENT)) continue;
		if (std::regex_match(line.c_str(), match, INI_REGEX_SECTION)) {
			if (curConfigs.size() > 0) {
				callbackPerSection(curSection, curConfigs);
				curConfigs.clear();
			}
			curSection = match[1];
			continue;
		}
		if (std::regex_match(line.c_str(), match, INI_REGEX_CONF)) {
			auto& confLineStream = curConfigs.emplace_back();
			Conf conf {this, match[1].str(), &confLineStream.value, STREAM};
			confLineStream.name = conf.name;
			conf.ReadValue(match[2].str());
			continue;
		}
		LOG_WARN_VERBOSE("Line " << n << " is invalid in config file " << filePath)
	}
	file.close();
	
	if (curConfigs.size() > 0) {
		callbackPerSection(curSection, curConfigs);
	}
}
