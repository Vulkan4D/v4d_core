/**
 * This helper is part of Vulkan4D (open source project)
 * 
 * @author Ivan Molodtsov, Olivier St-Laurent
 * @date 2019-06-18
 */
#pragma once

using namespace std;

namespace v4d {

    #define MILLIS_TO_SECOND 0.001

    typedef chrono::duration<double, milli> duration;
    typedef chrono::time_point<chrono::system_clock, duration> time_point;

    class Timer {
    private:
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
        inline void Start() {
            timePoint = chrono::high_resolution_clock::now();
        }

        /**
         * resets the time point to the current system time
         */
        inline void Reset() {
            Start();
        }

        /**
         * @returns the elapsed milliseconds between the current system time and the time point
         */
        inline double GetElapsedMilliseconds() const {
            return GetDurationSince().count();
        }

        /**
         * @returns the elapsed seconds between the current system time and the time point
         */
        inline double GetElapsedSeconds() const {
            return GetDurationSince().count() * MILLIS_TO_SECOND;
        }

	private:
        /**
         * @returns the duration between the current system time and the time point
         */
        inline duration GetDurationSince() const {
            return chrono::high_resolution_clock::now() - timePoint;
        }

    };
}
