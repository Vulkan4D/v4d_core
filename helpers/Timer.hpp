#pragma once

// #include <chrono>

namespace v4d {

	class Timer {
		typedef std::chrono::duration<double, std::milli> duration;
		typedef std::chrono::time_point<std::chrono::system_clock, duration> time_point;
		mutable std::mutex mutex;
		time_point timePoint;

	public:
		/**
		 * Default constructor, initialize timePoint to current system time
		 * @param startNow bool (starts the timer immediately)
		 */
		Timer(bool startNow = false) {
			if (startNow)
				Start();
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

	private:
		/**
		 * @returns the duration between the current system time and the time point
		 */
		duration GetDurationSince() const {
			return std::chrono::high_resolution_clock::now() - timePoint;
		}

	};
}
