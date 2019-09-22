#include "ThreadPool.h"

using namespace v4d::processing;

ThreadPool::ThreadPool(const size_t& numThreads) {
	stopping = false;
	SetNbThreads(numThreads);
}

void ThreadPool::StartNewThread(const size_t& index) {
	threads.emplace(index, 
		[this, index] {
			while(true) {
				try {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(eventMutex);
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
						task = move(this->tasks.front());
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
	{
		std::lock_guard<std::mutex> lock(eventMutex);
		stopping = true;
	}

	eventVar.notify_all();

	try {
		for (auto& thread : threads) {
			if (thread.second.joinable()) {
				thread.second.join();
			}
		}
	} catch (std::exception& e) {
		LOG_ERROR("Error while joining ThreadPool threads: " << e.what())
	} catch (...) {
		LOG_ERROR("Unknown Error while joining ThreadPool threads")
	}
}

void ThreadPool::SetNbThreads(const size_t& numThreads) {
	{
		std::lock_guard<std::mutex> lock(eventMutex);
		this->numThreads = numThreads;
		while (threads.size() < numThreads) {
			StartNewThread(threads.size());
		}
	}

	eventVar.notify_all();

	for (auto& thread : threads) {
		if (thread.first >= numThreads) {
			thread.second.join();
		}
	}
}

