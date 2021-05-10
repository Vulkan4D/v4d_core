#pragma once

#include <v4d.h>
#include <cmath>

namespace v4d {
	class FPSCounter {
		
		v4d::Timer avgTimer;
		double averagedTime; // seconds
		v4d::Timer deltaTimer;
		
		double avgFramerate = 0; // frames per second
		int nbFrames = 0;
		
	public:
		double innerFrameTime = 0; // only used when limiting framerate, otherwise deltaTime can be used
		double deltaTime = 0;
		
		FPSCounter(double averagedTime = 1.0)
		 : avgTimer(true), averagedTime(averagedTime), deltaTimer(true) {}
		
		FPSCounter& Tick(double minDeltaTime = 0.0001) {
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
		
		operator double() {
			return std::round(avgFramerate*10)*0.1;
		}
		
		operator int() {
			return int(std::round(avgFramerate));
		}
		
	};
}
