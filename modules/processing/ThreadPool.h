/**
 * A ThreadPool for asynchronous parallel execution on a defined number of threads.
 * The pool keeps a vector of threads alive, waiting on a condition variable for some work to become available.
 *
 * @author Ivan Molodtsov
 * @date 2019-06-20
 */
#pragma once

#include "v4d.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <vector>
#include <thread>
#include <queue>

using namespace std;

namespace v4d::processing {

    V4DLIB class ThreadPool
    {
    private:
        typedef function<void()> Task;

        vector<thread> threads;
        queue<Task> tasks;

        mutex eventMutex;
        condition_variable eventVar;

        bool stopping;

    public:
        /**
         * ThreadPool sole constructor
         * @param nbThreads size_t number of threads
         */
        ThreadPool(size_t nbThreads);

        /**
         * ThreadPool destructor
         * Frees all threads and tasks
         */
        ~ThreadPool();

        /**
         * Enqueue a task that will eventually be executed by one of the threads of the pool
         * @Params: task that returns no value
         */
        void enqueue(Task);

        /**
         * Enqueue a task and gives a promise to the task return value
         * @Params: task with a return value
         * @Returns: a promise for the returned value by the task 
         */
        template<typename T>
        auto promise(T task) -> std::future<decltype(task())>
        {
            auto wrapper = make_shared<packaged_task<decltype(task())()>>(move(task));
            
            enqueue(
                [wrapper]
                {
                    (*wrapper)();
                }
            );

            return wrapper->get_future();
        }
    };
}
