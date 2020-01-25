#pragma once
#include <v4d.h>

namespace v4d::processing {
	
	class ThreadPoolBase {
	protected:
		bool stopping = false;
		size_t numThreads = 0;
		mutable std::mutex eventMutex {}; // For stopping, numThreads and tasks
		std::unordered_map<index_t, std::thread> threads {};
		std::recursive_mutex threadsMutex {};
		std::condition_variable eventVar {};
		
		virtual void StartNewThread(index_t) = 0;
		
	public:

		/**
		 * ThreadPoolBase destructor
		 * Frees all threads and tasks
		 */
		virtual ~ThreadPoolBase() {
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

		/**
		 * Set a new number of threads
		 * @param number of threads
		 */
		virtual void RunThreads(size_t numThreads) {
			if (numThreads == this->numThreads) return;
			
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

	};
	
	template<class QueuedElementType = std::function<void()>, class QueueType = std::queue<QueuedElementType, std::deque<QueuedElementType>>>
	class ThreadPool : public ThreadPoolBase {
	protected:

		std::function<void(QueuedElementType& item)> taskRunFunction;
		QueueType items {};
		
		virtual void StartNewThread(index_t index) override {
			std::lock_guard threadsLock(threadsMutex);
			threads.emplace(index, 
				[this, index] {
					while(true) {
						try {
							QueuedElementType item;
							{
								std::unique_lock lock(eventMutex);
								eventVar.wait(lock, [this,index] {
									return this->stopping || index >= this->numThreads || !this->items.empty();
								});

								// End thread if threadpool is destroyed
								if (stopping) {
									break;
								}
								
								// End thread if we have reduced the number of threads in the pool
								if (index >= this->numThreads) {
									break;
								}

								// get the next item to execute
								if constexpr (std::is_same_v<QueueType, std::queue<QueuedElementType>>) {
									// std::queue
									item = std::move(this->items.front());
								} else {
									// std::priority_queue
									item = std::move(this->items.top());
								}
								this->items.pop();
							}

							taskRunFunction(item);
							
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

	public:
	
		ThreadPool(
			std::function<void(QueuedElementType& item)> perItemFunc = [](QueuedElementType& item){
				if constexpr (std::is_same_v<QueuedElementType, std::function<void()>>) {
					item();
				}
			}
		) : taskRunFunction(perItemFunc) {}
		
		size_t Count() const {
			std::lock_guard lock(eventMutex);
			return items.size();
		}
		
		/**
		 * Set the function to run for each item in the queue
		 * @Param function that returns no value
		 */
		void SetRunFunction(std::function<void(QueuedElementType& item)>&& func) {
			std::lock_guard lock(eventMutex);
			taskRunFunction = std::forward<std::function<void(QueuedElementType& item)>>(func);
		}
		
		/**
		 * Enqueue a task/item that will eventually be executed by one of the threads of the pool
		 * @Param task/item that returns no value
		 */
		void Enqueue(QueuedElementType item) {
			{
				std::lock_guard lock(eventMutex);
				if (stopping) {
					return;
				}
				items.emplace(item);
			}
			eventVar.notify_all();
		}
		
		/**
		 * Enqueue a task and gives a promise to the task return value
		 * @Param task with a return value
		 * @Returns: a promise for the returned value by the task 
		 */
		template<typename T>
		auto Promise(T task) -> std::future<decltype(task())> {
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
		auto Promise(T task, uint delayMilliseconds) -> std::future<std::future<decltype(task())>> {
			return Promise<T, uint, std::milli>(std::move(task), std::chrono::milliseconds{delayMilliseconds});
		}
		template<typename T, typename rep, typename period>
		auto Promise(T task, const std::chrono::duration<rep, period>& delay) -> std::future<std::future<decltype(task())>> {
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
	
	#define THREADPOOL_PRIORITYQUEUE_TYPE std::priority_queue<QueuedElementType, std::vector<QueuedElementType>, std::function<bool(QueuedElementType& a, QueuedElementType& b)>>
	#define THREADPOOL_PRIORITYQUEUE_TEMPLATE ThreadPool<QueuedElementType, THREADPOOL_PRIORITYQUEUE_TYPE>

	template<class QueuedElementType>
	class ThreadPoolPriorityQueue : public THREADPOOL_PRIORITYQUEUE_TEMPLATE {
	public:
	
		ThreadPoolPriorityQueue(
			std::function<void(QueuedElementType& item)> perItemFunc, 
			std::function<bool(QueuedElementType& a, QueuedElementType& b)> queueParam
		) : THREADPOOL_PRIORITYQUEUE_TEMPLATE(perItemFunc) {
			THREADPOOL_PRIORITYQUEUE_TEMPLATE::items = THREADPOOL_PRIORITYQUEUE_TYPE{queueParam};
		}
		
	};
}
