#ifndef __PHYSICS_ANALYSIS_HPP__
#define __PHYSICS_ANALYSIS_HPP__

#include <chrono>
#include <deque>

class PhysicsAnalysis {

public:

	PhysicsAnalysis(size_t samplesToAverage) : maxSamples(samplesToAverage) { }

	void reset();

	void logPosition(int pos);

	float getAveragePosition() const;

	float getAverageVelocity() const;

	float getAverageAcceleration() const;

	bool hasPosition() const { return !positionLog.empty(); }

	bool hasVelocity() const { return !velocityLog.empty(); }

	bool hasAcceleration() const { return !accelLog.empty(); }

	PhysicsAnalysis(const PhysicsAnalysis&) = delete;
	PhysicsAnalysis& operator=(const PhysicsAnalysis&) = delete;

private:

	typedef std::chrono::high_resolution_clock Clock;

	struct Entry {
		Entry(float v) : val(v), time(Clock::now()) { }

		const float val;
		const Clock::time_point time;
	};

	// Should replace with a circular buffer
	std::deque<Entry> positionLog;
	std::deque<Entry> velocityLog;
	std::deque<Entry> accelLog;

	const size_t maxSamples;

};

#endif
