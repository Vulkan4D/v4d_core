#include "ThreadPool.h"

using namespace v4d::processing;

ThreadPoolBase::ThreadPoolBase(const char* name) : name(name) {
	assert(strlen(name) < 16);
}

ThreadPoolBase::~ThreadPoolBase() {
	Shutdown();
}

void ThreadPoolBase::RunThreads(size_t numThreads, std::function<void()> initPerThread) {
	if (numThreads == this->numThreads) return;
	
	std::lock_guard threadsLock(threadsMutex);
	
	{
		std::lock_guard lock(eventMutex);
		this->numThreads = numThreads;
		while (threads.size() < numThreads) {
			StartNewThread(threads.size(), initPerThread);
		}
	}

	eventVar.notify_all();

	for (auto& [index, thread] : threads) {
		if (index >= numThreads && thread.joinable()) {
			thread.join();
		}
	}
}

void ThreadPoolBase::Shutdown() {
	std::lock_guard threadsLock(threadsMutex);
	
	{
		std::lock_guard lock(eventMutex);
		stopping = true;
		numThreads = 0;
	}

	eventVar.notify_all();

	try {
		for (auto& [index, thread] : threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}
	} catch (std::exception& e) {
		// LOG_ERROR("Error while joining ThreadPool threads: " << e.what())
	} catch (...) {
		// LOG_ERROR("Unknown Error while joining ThreadPool threads")
	}
	std::lock_guard lock(eventMutex);
}
