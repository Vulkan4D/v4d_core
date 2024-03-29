#pragma once

#include <chrono>
#include <mutex>

namespace v4d {

	class Timer {
		typedef std::chrono::duration<double, std::milli> duration;
		typedef std::chrono::time_point<std::chrono::system_clock, duration> time_point;
		mutable std::mutex mutex;
		time_point timePoint {};

	public:
		/**
		 * Default constructor, initialize timePoint to current system time
		 * @param startNow bool (starts the timer immediately)
		 */
		Timer(bool startNow = false) {
			if (startNow)
				Start();
		}

		Timer(const Timer& other) {
			timePoint = other.timePoint;
		}
		Timer(Timer&& other) {
			timePoint = other.timePoint;
		}
		Timer& operator= (const Timer& other) {
			timePoint = other.timePoint;
			return *this;
		}
		Timer& operator= (Timer&& other) {
			timePoint = other.timePoint;
			return *this;
		}

		/**
		 * starts the time point to the current system time
		 */
		void Start() {
			std::lock_guard lock(mutex);
			timePoint = std::chrono::high_resolution_clock::now();
		}

		/**
		 * resets the time point to the current system time
		 */
		void Reset() {
			Start();
		}

		/**
		 * @returns the elapsed milliseconds between the current system time and the time point
		 */
		double GetElapsedMilliseconds() const {
			std::lock_guard lock(mutex);
			return GetDurationSince().count();
		}

		/**
		 * @returns the elapsed seconds between the current system time and the time point
		 */
		double GetElapsedSeconds() const {
			return GetElapsedMilliseconds() * 0.001;
		}
		
		static double GetCurrentTimestamp() {
			time_point time = std::chrono::high_resolution_clock::now();
			return time.time_since_epoch().count() * 0.001;
		}

	private:
		/**
		 * @returns the duration between the current system time and the time point
		 */
		duration GetDurationSince() const {
			return std::chrono::high_resolution_clock::now() - timePoint;
		}

	};
}
