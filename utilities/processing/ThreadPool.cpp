#include <v4d.h>

using namespace v4d::processing;

ThreadPool::ThreadPool(size_t numThreads) : stopping(false) {
	SetNbThreads(numThreads);
}

void ThreadPool::StartNewThread(index_t index) {
	std::lock_guard threadsLock(threadsMutex);
	threads.emplace(index, 
		[this, index] {
			while(true) {
				try {
					std::function<void()> task;
					{
						std::unique_lock lock(eventMutex);
						eventVar.wait(lock, [this,index] {
							return this->stopping || index >= this->numThreads || !this->tasks.empty();
						});

						// End thread if threadpool is destroyed
						if (stopping) {
							break;
						}
						
						// End thread if we have reduced the number of threads in the pool
						if (index >= this->numThreads) {
							break;
						}

						// get the next task to execute
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();
				} catch (std::exception& e) {
					LOG_ERROR("Error in a ThreadPool task: " << e.what())
					SLEEP(100ms)
				} catch (...) {
					LOG_ERROR("Unknown Error in a ThreadPool task")
					SLEEP(100ms)
				}
			}
		}
	);
}

ThreadPool::~ThreadPool() {
	std::lock_guard threadsLock(threadsMutex);
	
	{
		std::lock_guard lock(eventMutex);
		stopping = true;
	}

	eventVar.notify_all();

	try {
		for (auto& [index, thread] : threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}
	} catch (std::exception& e) {
		LOG_ERROR("Error while joining ThreadPool threads: " << e.what())
	} catch (...) {
		LOG_ERROR("Unknown Error while joining ThreadPool threads")
	}
	std::lock_guard lock(eventMutex);
}

void ThreadPool::SetNbThreads(size_t numThreads) {
	std::lock_guard threadsLock(threadsMutex);

	{
		std::lock_guard lock(eventMutex);
		this->numThreads = numThreads;
		while (threads.size() < numThreads) {
			StartNewThread(threads.size());
		}
	}

	eventVar.notify_all();

	for (auto& [index, thread] : threads) {
		if (index >= numThreads && thread.joinable()) {
			thread.join();
		}
	}
}

