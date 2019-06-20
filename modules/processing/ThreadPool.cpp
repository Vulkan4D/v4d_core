#include "ThreadPool.h"

using namespace v4d::processing;

ThreadPool::ThreadPool(size_t numThreads) 
{
    stopping = false;

    for (size_t i=0; i<numThreads; i++) 
    {
        threads.emplace_back(
            [this]
            {
                while(true) 
                {
                    Task task;

                    {
                        unique_lock<mutex> lock(eventMutex);
                        eventVar.wait(lock, [this] {return this->stopping || !this->tasks.empty();});

                        if(stopping)
                            break;

                        task = move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
    }
}

ThreadPool::~ThreadPool() 
{
    {
        unique_lock<mutex> lock(eventMutex);
        stopping = true;
    }

    eventVar.notify_all();

    for (auto& thread : threads)
        thread.join();
}

void ThreadPool::enqueue(Task task) 
{
    {
        unique_lock<mutex> lock(eventMutex);

        if (stopping)
            return;

        tasks.emplace(task);
    }

    eventVar.notify_one();
}
