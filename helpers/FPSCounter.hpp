#pragma once

#include <v4d.h>
#include <cmath>
#include <sstream>
#include <atomic>

namespace v4d {
	class FPSCounter {
		
		v4d::Timer avgTimer;
		v4d::Timer deltaTimer;
		double averagedTime; // seconds
		
		std::atomic<double> avgFramerate = 0; // frames per second
		int nbFrames = 0;
		
	public:
		double innerFrameTime = 0; // only used when limiting framerate, otherwise deltaTime can be used
		double deltaTime = 0;
		
		FPSCounter(double averagedTime = 1.0)
		 : avgTimer(true), deltaTimer(true), averagedTime(averagedTime) {}
		
		FPSCounter& Tick() {
			++nbFrames;
			deltaTime = deltaTimer.GetElapsedSeconds();
			const double elapsedTime = avgTimer.GetElapsedSeconds();
			if (elapsedTime > averagedTime) {
				avgFramerate = nbFrames / elapsedTime;
				nbFrames = 0;
				avgTimer.Reset();
			}
			deltaTimer.Reset();
			return *this;
		}
		
		void Limit(double frameRate) {
			if (frameRate > 0.0) {
				innerFrameTime = deltaTimer.GetElapsedSeconds();
				const double timeToSleep = 1.0/frameRate - innerFrameTime;
				if (timeToSleep > 0.0001) {
					SLEEP(1.0s * timeToSleep)
				}
			}
		}
		
		operator double() const {
			return avgFramerate;
		}
		
		operator int() const {
			return int(std::round(avgFramerate));
		}
		
		operator std::string () const {
			std::stringstream out {};
			out.precision(1);
			out << std::fixed << (std::round(avgFramerate*10)*0.1);
			return out.str();
		}

	};
}

#define V4D_LIMIT_FRAMERATE(timer, maxFramerate) {\
	double sleepTime = (1.0 / maxFramerate) - timer.GetElapsedSeconds();\
	if (sleepTime > 0.0001) SLEEP(1s * sleepTime)\
	timer.Reset();\
}
