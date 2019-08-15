#pragma once

#include "ThreadPool.h"

namespace v4d::tests {
	int ThreadPool() {
		auto timer = v4d::Timer(true);

		int result = 0;
		{
			v4d::processing::ThreadPool threadPool(3);

			// Task 1
			auto future1 = threadPool.Promise([&timer] {
				SLEEP(2s)
				return 111;
			});
			if (timer.GetElapsedMilliseconds() > 100) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.1 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 2
			auto future2 = threadPool.Promise([&timer] {
				SLEEP(1s)
				return 222;
			});
			if (timer.GetElapsedMilliseconds() > 100) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.2 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 3
			auto future3 = threadPool.Promise([&timer] {
				SLEEP(3s)
				return 333;
			});
			if (timer.GetElapsedMilliseconds() > 100) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.3 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 4
			threadPool.Enqueue([&timer] {
				SLEEP(4s)
			}, 1000);
			if (timer.GetElapsedMilliseconds() > 100) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.4 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 5
			auto future5 = threadPool.Promise([&future1, &future2, &future3, &timer] {
				return future1.get() + future2.get() + future3.get();
			}, 4000); // delay of 4 seconds
			if (timer.GetElapsedMilliseconds() > 100) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.5 (task not enqueued asynchronously)")
				return 1;
			}

			// Task 6
			auto future6 = threadPool.Promise([&timer] {
				return timer.GetElapsedSeconds();
			}, 1000); // delay of 1 second
			if (timer.GetElapsedMilliseconds() > 100) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 1.6 (task not enqueued asynchronously)")
				return 1;
			}

			result = future5.get().get();

			double task6Result = future6.get().get();
			if (task6Result < 1 || task6Result > 2.1) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 2 (task 6 executed after " << task6Result << " seconds instead of ~1.0-2.1)")
				return 1;
			}
		}

		if (result != 666) {
			LOG_ERROR("v4d::tests::ThreadPool ERROR 3 (resulting value was " << result << " instead of 666)")
			return 1;
		}

		double elapsedTime = timer.GetElapsedSeconds();
		if (elapsedTime < 5 || elapsedTime > 5.1) {
			LOG_ERROR("v4d::tests::ThreadPool ERROR 4 (execution time was " << elapsedTime << " seconds instead of ~5.0-5.1 seconds)")
			return 1;
		}

		// Second batch of tests (changing thread count at runtime)
		{
			timer.Reset();
			v4d::processing::ThreadPool threadPool(1);
			{
				auto t1 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				auto t2 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				t1.get();
				t2.get();
			}
			elapsedTime = timer.GetElapsedSeconds();
			if (elapsedTime < 2 || elapsedTime > 2.1) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 5 (execution time was " << elapsedTime << " seconds instead of ~2.0-2.1 seconds)")
				return 1;
			}

			timer.Reset();
			threadPool.SetNbThreads(3);
			{
				auto t1 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				auto t2 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				auto t3 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				t1.get();
				t2.get();
				t3.get();
			}
			elapsedTime = timer.GetElapsedSeconds();
			if (elapsedTime < 1 || elapsedTime > 1.1) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 6 (execution time was " << elapsedTime << " seconds instead of ~1.0-1.1 seconds)")
				return 1;
			}

			timer.Reset();
			threadPool.SetNbThreads(2);
			{
				auto t1 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				auto t2 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				auto t3 = threadPool.Promise([] {
					SLEEP(1s)
					return 0;
				});
				t1.get();
				t2.get();
				t3.get();
			}
			elapsedTime = timer.GetElapsedSeconds();
			if (elapsedTime < 2 || elapsedTime > 2.1) {
				LOG_ERROR("v4d::tests::ThreadPool ERROR 7 (execution time was " << elapsedTime << " seconds instead of ~2.0-2.1 seconds)")
				return 1;
			}

		}

		// SUCCESS
		return 0;
	}
}
