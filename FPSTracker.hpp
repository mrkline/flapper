#ifndef __FPS_TRACKER_HPP__
#define __FPS_TRACKER_HPP__

#include <chrono>
#include <mutex>
#include <queue>

/// A crude perforamnce measure
class FPSTracker {

public:

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
		std::lock_guard<std::mutex> lg(printMutex);
		if (Clock::now() >= lastPrinted + std::chrono::seconds(1))
		{
			printf("%s%d\n", message, getFPS());
			fflush(stdout);
			lastPrinted = Clock::now();
		}
	}

private:

	typedef std::chrono::high_resolution_clock Clock;

	std::queue<Clock::time_point> framesInLastSecond;
	std::mutex queueMutex;
	Clock::time_point lastPrinted;
	std::mutex printMutex;
};

#endif
