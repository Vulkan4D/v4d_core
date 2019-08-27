#pragma once

// #include <chrono>

namespace v4d {

	class Timer {
		typedef std::chrono::duration<double, std::milli> duration;
		typedef std::chrono::time_point<std::chrono::system_clock, duration> time_point;

	private:
		time_point timePoint;

	public:
		/**
		 * Default constructor, initialize timePoint to current system time
		 * @param startNow bool (starts the timer immediately)
		 */
		Timer(const bool& startNow = false) {
			if (startNow)
				Start();
		}

		/**
		 * starts the time point to the current system time
		 */
		INLINE void Start() {
			timePoint = std::chrono::high_resolution_clock::now();
		}

		/**
		 * resets the time point to the current system time
		 */
		INLINE void Reset() {
			Start();
		}

		/**
		 * @returns the elapsed milliseconds between the current system time and the time point
		 */
		INLINE double GetElapsedMilliseconds() const {
			return GetDurationSince().count();
		}

		/**
		 * @returns the elapsed seconds between the current system time and the time point
		 */
		INLINE double GetElapsedSeconds() const {
			return GetElapsedMilliseconds() * 0.001;
		}

	private:
		/**
		 * @returns the duration between the current system time and the time point
		 */
		INLINE duration GetDurationSince() const {
			return std::chrono::high_resolution_clock::now() - timePoint;
		}

	};
}
