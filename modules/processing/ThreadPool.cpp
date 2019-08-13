#include "ThreadPool.h"

using namespace v4d::processing;

ThreadPool::ThreadPool(size_t numThreads) {
    stopping = false;
    SetNbThreads(numThreads);
}

void ThreadPool::StartNewThread(size_t index) {
    threads.emplace(index, 
        [this, index] {
            while(true) {
                task task;
                {
                    unique_lock<mutex> lock(eventMutex);
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

                try {
                    task();
                } catch (std::exception& e) {
                    LOG_ERROR("Error in a ThreadPool task: " << e.what())
                } catch (...) {
                    LOG_ERROR("Unknown Error in a ThreadPool task")
                }
            }
        }
    );
}

ThreadPool::~ThreadPool() {
    {
        unique_lock<mutex> lock(eventMutex);
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

void ThreadPool::SetNbThreads(size_t numThreads) {
    {
        unique_lock<mutex> lock(eventMutex);
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

void ThreadPool::Enqueue(task t) {
    {
        unique_lock<mutex> lock(eventMutex);
        if (stopping) {
            return;
        }
        tasks.emplace(t);
    }

    eventVar.notify_all();
}

void ThreadPool::Enqueue(task task, unsigned int delayMilliseconds) {
    std::thread([delayMilliseconds, &task, this] {
        // Delay
        if (delayMilliseconds > 0) std::this_thread::sleep_for(std::chrono::milliseconds{delayMilliseconds});

        Enqueue(task);
        
    }).detach();
}
