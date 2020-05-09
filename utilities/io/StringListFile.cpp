#include <v4d.h>

using namespace v4d::io;


int StringListFile::globalAutoReloadInterval = DEFAULT_AUTORELOAD_INTERVAL;

StringListFile::StringListFile(const std::string& filePath, std::optional<int> autoReloadInterval) : FilePath(filePath), autoReloadInterval(autoReloadInterval.value_or(globalAutoReloadInterval)) {
	AutoCreateFile();
	if (autoReloadInterval > 0) {
		StartAutoReloadThread();
	}
}

StringListFile::~StringListFile() {
	autoReloadInterval = 0;
	{
		std::lock_guard lock(mu);
	}
	if (autoReloadThread && autoReloadThread->joinable()) {
		autoReloadThread->join();
		delete autoReloadThread;
	}
}

std::vector<std::string>& StringListFile::Load() {
	Load([](StringListFile*){});
	return lines;
}

void StringListFile::Load(ReloadCallbackFunc&& callback) {
	std::lock_guard lock(mu);
	lastWriteTimeCache = GetLastWriteTime();
	if (lastWriteTimeCache == 0) {
		AutoCreateFile();
		lastWriteTimeCache = GetLastWriteTime();
	}
	ReadLinesFromFile();
	lastWriteTimeCache = GetLastWriteTime();
	reloadCallback = std::forward<ReloadCallbackFunc>(callback);
	reloadCallback(this);
}

void StringListFile::SetAutoReloadInterval(int interval) {
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

bool StringListFile::FileHasChanged() const {
	std::lock_guard lock(mu);
	return lastWriteTimeCache != GetLastWriteTime();
}

void StringListFile::StartAutoReloadThread() {
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
					ReadLinesFromFile();
					lastWriteTimeCache = GetLastWriteTime();
					reloadCallback(this);
				}
			}
			interval = autoReloadInterval;
		}
	});
}

void StringListFile::ReadLinesFromFile() {
	LOG_VERBOSE("String List File Read: " << filePath.string())
	
	std::ifstream file(filePath);
	std::string line;
	while (std::getline(file, line)) {
		v4d::String::Trim(line);
		if (line.length() == 0) continue;
		lines.push_back(line);
	}
	file.close();
}

void StringListFile::WriteLinesToFile() {
	LOG_VERBOSE("String List File Write: " << filePath.string())

	// Write all lines to file
	std::ofstream file(filePath);
	for (auto line : lines) {
		file << line << "\r\n";
	}
	file.close();
}
