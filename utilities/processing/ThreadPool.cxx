#pragma once

#include "ThreadPool.h"

namespace v4d::tests {
	int ThreadPool() {
		auto timer = v4d::Timer(true);

		int result = 0;
		{
			v4d::processing::ThreadPool threadPool(3);

			using namespace std::literals::chrono_literals;

			// Task 1
			auto future1 = threadPool.Promise([&timer] {
				SLEEP(200ms)
				return 111;
			});
			if (timer.GetElapsedMilliseconds() > 10) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.1 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 2
			auto future2 = threadPool.Promise([&timer] {
				SLEEP(100ms)
				return 222;
			});
			if (timer.GetElapsedMilliseconds() > 10) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.2 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 3
			auto future3 = threadPool.Promise([&timer] {
				SLEEP(300ms)
				return 333;
			});
			if (timer.GetElapsedMilliseconds() > 10) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.3 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 4
			threadPool.Enqueue([&timer] {
				SLEEP(400ms)
			}, 100);
			if (timer.GetElapsedMilliseconds() > 10) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.4 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 5
			auto future5 = threadPool.Promise([&future1, &future2, &future3, &timer] {
				try {
					return future1.get() + future2.get() + future3.get();
				} catch (...) {
					return 0;
				}
			}, 400ms); // delay of 400 milliseconds
			if (timer.GetElapsedMilliseconds() > 10) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.5 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 6
			auto future6 = threadPool.Promise([&timer] {
				return timer.GetElapsedMilliseconds();
			}, 100ms); // delay of 100 milliseconds
			if (timer.GetElapsedMilliseconds() > 10) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.6 (task not enqueued asynchronously)")
				return 1;
			}

			try {
				result = future5.get().get();
			} catch (std::exception& e) {
				LOG_ERROR("v4d::tests::ThreadPool EXCEPTION 1 : " << e.what())
			} catch (...) {
				LOG_ERROR("v4d::tests::ThreadPool UNKNOWN EXCEPTION 1")
			}

			try {
				double task6Result = future6.get().get();
				if (task6Result < 100 || task6Result > 210) {
					LOG_ERROR("v4d::tests::ThreadPool ERROR 2 (task 6 executed after " << task6Result << " milliseconds instead of ~100-210)")
					return 1;
				}
			} catch (std::exception& e) {
				LOG_ERROR("v4d::tests::ThreadPool EXCEPTION 2 : " << e.what())
			} catch (...) {
				LOG_ERROR("v4d::tests::ThreadPool UNKNOWN EXCEPTION 2")
			}
		}

		if (result != 666) {
			LOG_ERROR("v4d::tests::ThreadPool ERROR 3 (resulting value was " << result << " instead of 666)")
			return 1;
		}

		double elapsedTime = timer.GetElapsedMilliseconds();
		if (elapsedTime < 500 || elapsedTime > 510) {
			LOG_ERROR("v4d::tests::ThreadPool ERROR 4 (execution time was " << elapsedTime << " milliseconds instead of ~500-510)")
			return 1;
		}

		// Second batch of tests (changing thread count at runtime)
		{
			timer.Reset();
			v4d::processing::ThreadPool threadPool(1);
			{
				auto t1 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				auto t2 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				try {
					t1.get();
					t2.get();
				} catch (std::exception& e) {
					LOG_ERROR("v4d::tests::ThreadPool EXCEPTION 3 : " << e.what())
				} catch (...) {
					LOG_ERROR("v4d::tests::ThreadPool UNKNOWN EXCEPTION 3")
				}
			}
			elapsedTime = timer.GetElapsedMilliseconds();
			if (elapsedTime < 200 || elapsedTime > 210) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 5 (execution time was " << elapsedTime << " milliseconds instead of ~200-210)")
				return 1;
			}

			timer.Reset();
			threadPool.SetNbThreads(3);
			{
				auto t1 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				auto t2 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				auto t3 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				try {
					t1.get();
					t2.get();
					t3.get();
				} catch (std::exception& e) {
					LOG_ERROR("v4d::tests::ThreadPool EXCEPTION 4 : " << e.what())
				} catch (...) {
					LOG_ERROR("v4d::tests::ThreadPool UNKNOWN EXCEPTION 4")
				}
			}
			elapsedTime = timer.GetElapsedMilliseconds();
			if (elapsedTime < 100 || elapsedTime > 110) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 6 (execution time was " << elapsedTime << " milliseconds instead of ~100-110)")
				return 1;
			}

			timer.Reset();
			threadPool.SetNbThreads(2);
			{
				auto t1 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				auto t2 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				auto t3 = threadPool.Promise([] {
					SLEEP(100ms)
					return 0;
				});
				try {
					t1.get();
					t2.get();
					t3.get();
				} catch (std::exception& e) {
					LOG_ERROR("v4d::tests::ThreadPool EXCEPTION 5 : " << e.what())
				} catch (...) {
					LOG_ERROR("v4d::tests::ThreadPool UNKNOWN EXCEPTION 5")
				}
			}
			elapsedTime = timer.GetElapsedMilliseconds();
			if (elapsedTime < 200 || elapsedTime > 210) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 7 (execution time was " << elapsedTime << " milliseconds instead of ~200-210)")
				return 1;
			}

		}

		// SUCCESS
		return 0;
	}
}
