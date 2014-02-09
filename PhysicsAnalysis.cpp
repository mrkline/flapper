#include "PhysicsAnalysis.hpp"

#include "Exceptions.hpp"

void PhysicsAnalysis::reset()
{
	positionLog.clear();
	velocityLog.clear();
	accelLog.clear();
}

void PhysicsAnalysis::logPosition(int pos)
{
	typedef std::chrono::duration<float, std::chrono::seconds::period> FloatingSeconds;
	static_assert(
		std::chrono::treat_as_floating_point<FloatingSeconds::rep>::value, "Rep required to be floating point"
	);

	positionLog.emplace_front(pos);

	if (positionLog.size() >= 2) {
		const Entry& now = positionLog[0];
		const Entry& then = positionLog[1];
		const float dt = FloatingSeconds(now.time - then.time).count(); // in seconds
		// dx is a height value, but we want to preserve the traditional dx/dt notation
		const float dx = now.val - then.val;
		velocityLog.emplace_front(dx/dt); // pixels/second
	}
	if (velocityLog.size() >= 2)
	{
		const Entry& now = velocityLog[0];
		const Entry& then = velocityLog[1];
		const float dt2 = FloatingSeconds(now.time - then.time).count(); // in seconds^2
		const float d2x = now.val - then.val;
		accelLog.emplace_front(d2x/dt2);
	}

	while (positionLog.size() >= maxSamples)
		positionLog.pop_back();
	while (velocityLog.size() >= maxSamples)
		velocityLog.pop_back();
	while (accelLog.size() >= maxSamples)
		accelLog.pop_back();
}

float PhysicsAnalysis::getAveragePosition() const
{
	if (positionLog.empty()) {
		throw Exceptions::InvalidOperationException("At least one position must be logged before getting position.",
		                                           __FUNCTION__);
	}

	float loc = 0;
	for (const auto& e : positionLog)
		loc += e.val;
	loc /= positionLog.size();
	return loc;
}

float PhysicsAnalysis::getAverageVelocity() const
{
	if (velocityLog.empty()) {
		throw Exceptions::InvalidOperationException("At least two positions must be logged before getting velocity.",
		                                           __FUNCTION__);
	}

	float vel = 0;
	for (const auto& e : velocityLog)
		vel += e.val;
	vel /= velocityLog.size();
	return vel;
}

float PhysicsAnalysis::getAverageAcceleration() const
{
	if (accelLog.empty()) {
		throw Exceptions::InvalidOperationException("At least three positions must be logged before getting acceleration.",
		                                           __FUNCTION__);
	}

	float accel = 0;
	for (const auto& e : accelLog)
		accel += e.val;
	accel /= accelLog.size();
	return accel;
}

