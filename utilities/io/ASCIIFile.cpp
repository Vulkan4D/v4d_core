#include "ASCIIFile.h"

using namespace v4d::io;

int ASCIIFile::globalAutoReloadInterval = DEFAULT_AUTORELOAD_INTERVAL;

ASCIIFile::ASCIIFile(const std::string& filePath, std::optional<int> autoReloadInterval) : FilePath(filePath), autoReloadInterval(autoReloadInterval.value_or(globalAutoReloadInterval)) {
	AutoCreateFile();
	if (autoReloadInterval > 0) {
		StartAutoReloadThread();
	}
}

ASCIIFile::~ASCIIFile() {
	autoReloadInterval = 0;
	{
		std::lock_guard lock(mu);
	}
	if (autoReloadThread && autoReloadThread->joinable()) {
		autoReloadThread->join();
		delete autoReloadThread;
	}
}

void ASCIIFile::Load(ReloadCallbackFunc&& callback) {
	std::lock_guard lock(mu);
	lastWriteTimeCache = GetLastWriteTime();
	if (lastWriteTimeCache == 0) {
		AutoCreateFile();
		lastWriteTimeCache = GetLastWriteTime();
	}
	ReadFromFile();
	lastWriteTimeCache = GetLastWriteTime();
	reloadCallback = std::forward<ReloadCallbackFunc>(callback);
	reloadCallback(this);
}

void ASCIIFile::SetAutoReloadInterval(int interval) {
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

bool ASCIIFile::FileHasChanged() const {
	std::lock_guard lock(mu);
	return lastWriteTimeCache != GetLastWriteTime();
}

void ASCIIFile::StartAutoReloadThread() {
	autoReloadThread = new std::thread([this]{
		THREAD_NAME("ASCIIAutoRload")
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
					ReadFromFile();
					lastWriteTimeCache = GetLastWriteTime();
					reloadCallback(this);
				}
			}
			interval = autoReloadInterval;
		}
	});
}
