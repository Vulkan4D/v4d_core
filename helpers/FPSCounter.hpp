#pragma once

#include <v4d.h>
#include <cmath>

namespace v4d {
	class FPSCounter {
		
		v4d::Timer timer;
		double averagedTime; // seconds
		
		double avgFramerate = 0; // frames per second
		int nbFrames = 0;
		
	public:
		
		FPSCounter(double averagedTime = 1.0)
		 : timer(true), averagedTime(averagedTime) {}
		
		FPSCounter& Tick() {
			++nbFrames;
			double elapsedTime = timer.GetElapsedSeconds();
			if (elapsedTime > averagedTime) {
				avgFramerate = nbFrames / elapsedTime;
				nbFrames = 0;
				timer.Reset();
			}
			return *this;
		}
		
		operator double() {
			return std::round(avgFramerate*10)*0.1;
		}
		
		operator int() {
			return int(std::round(avgFramerate));
		}
		
	};
}
