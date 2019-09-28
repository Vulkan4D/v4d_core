#pragma once

#include "v4d.h"

namespace v4d::processing {

	class V4DLIB ThreadPool {
	protected:
		std::unordered_map<index_t, std::thread> threads;
		std::queue<std::function<void()>> tasks;

		std::mutex eventMutex;
		std::condition_variable eventVar;

		bool stopping;
		size_t numThreads;

		void StartNewThread(index_t);

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
		inline void Enqueue(std::function<void()> task) {
			{
				std::lock_guard<std::mutex> lock(eventMutex);
				if (stopping) {
					return;
				}
				tasks.emplace(task);
			}
			eventVar.notify_all();
		}

		/**
		 * Enqueue a task to be executed with a delay of at least n milliseconds
		 * @Param task that returns no value
		 * @Param delay in milliseconds
		 */
		inline void Enqueue(std::function<void()> task, uint delayMilliseconds) {
			Enqueue<uint, std::milli>(std::move(task), std::chrono::milliseconds{delayMilliseconds});
		}
		template<typename rep, typename period>
		inline void Enqueue(std::function<void()> task, const std::chrono::duration<rep, period>& delay) {
			std::thread([this, task, delay] {
				// Delay
				std::this_thread::sleep_for<rep, period>(delay);

				Enqueue(task);
				
			}).detach();
		}

		/**
		 * Enqueue a task and gives a promise to the task return value
		 * @Param task with a return value
		 * @Returns: a promise for the returned value by the task 
		 */
		template<typename T>
		inline auto Promise(T task) -> std::future<decltype(task())> {
			auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
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
		inline auto Promise(T task, uint delayMilliseconds) -> std::future<std::future<decltype(task())>> {
			return Promise<T, uint, std::milli>(std::move(task), std::chrono::milliseconds{delayMilliseconds});
		}
		template<typename T, typename rep, typename period>
		inline auto Promise(T task, const std::chrono::duration<rep, period>& delay) -> std::future<std::future<decltype(task())>> {
			auto f = std::async(std::launch::async, [this, task, delay] {
				// Delay
				std::this_thread::sleep_for<rep, period>(delay);

				auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
				
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
