/**
 * A ThreadPool for asynchronous parallel execution on a defined number of threads.
 * The pool keeps a vector of threads alive, waiting on a condition variable for some work to become available.
 *
 * @author Ivan Molodtsov, Olivier St-Laurent
 * @date 2019-06-20
 */
#pragma once

#include "v4d.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <thread>
#include <queue>

using namespace std;

namespace v4d::processing {

    V4DLIB class ThreadPool {
    protected:
        typedef function<void()> task;

        map<size_t, thread> threads;
        queue<task> tasks;

        mutex eventMutex;
        condition_variable eventVar;

        bool stopping;
        size_t numThreads;

        void StartNewThread(size_t index);

    public:

        /**
         * ThreadPool sole constructor
         * @param number of threads
         */
        ThreadPool(size_t numThreads);

        /**
         * ThreadPool destructor
         * Frees all threads and tasks
         */
        ~ThreadPool();

        /**
         * Set a new number of threads
         * @param number of threads
         */
        void SetNbThreads(size_t numThreads);

        /**
         * Enqueue a task that will eventually be executed by one of the threads of the pool
         * @Param task that returns no value
         */
        void Enqueue(task);

        /**
         * Enqueue a task to be executed with a delay of at least n milliseconds
         * @Param task that returns no value
         * @Param delay in milliseconds
         */
        void Enqueue(task, unsigned int delayMilliseconds);

        /**
         * Enqueue a task and gives a promise to the task return value
         * @Param task with a return value
         * @Returns: a promise for the returned value by the task 
         */
        template<typename T>
        auto Promise(T task) -> future<decltype(task())> {
            auto wrapper = make_shared<packaged_task<decltype(task())()>>(move(task));
            Enqueue(
                [wrapper] {
                    (*wrapper)();
                }
            );
            return wrapper->get_future();
        }
        
        /**
         * Enqueue a task to be executed with a delay of at least n milliseconds
         * @Param task that returns no value
         * @Param delay in milliseconds
         */
        template<typename T>
        auto Promise(T task, unsigned int delayMilliseconds) -> future<future<decltype(task())>> {
            auto f = async(launch::async, [this,delayMilliseconds,task] {
                // Delay
                if (delayMilliseconds > 0) std::this_thread::sleep_for(std::chrono::milliseconds{delayMilliseconds});

                auto wrapper = make_shared<packaged_task<decltype(task())()>>(move(task));
                
                Enqueue(
                    [wrapper] {
                        (*wrapper)();
                    }
                );

                return wrapper->get_future();
            });
            
            return f;
        }

    };
}
