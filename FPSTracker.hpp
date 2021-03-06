#ifndef __FPS_TRACKER_HPP__
#define __FPS_TRACKER_HPP__

#include <chrono>
#include <mutex>
#include <queue>

#include "PeriodicRunner.hpp"

/// A crude perforamnce measure
class FPSTracker {

public:

	FPSTracker() : runner(1) { }

	void onFrame()
	{
		std::lock_guard<std::mutex> lg(queueMutex);
		framesInLastSecond.push(Clock::now());
	}

	int getFPS()
	{
		auto then = Clock::now() - std::chrono::seconds(1);
		std::lock_guard<std::mutex> lg(queueMutex);
		while (!framesInLastSecond.empty() && framesInLastSecond.front() < then)
			framesInLastSecond.pop();
		return (int)framesInLastSecond.size();
	}

	void printPeriodically(const char* message)
	{
		runner.runPeriodically([message, this]() {
			printf("%s%d\n", message, getFPS());
			fflush(stdout);
		});
	}

private:

	typedef std::chrono::high_resolution_clock Clock;

	std::queue<Clock::time_point> framesInLastSecond;
	std::mutex queueMutex;
	PeriodicRunner<> runner;
};

#endif
