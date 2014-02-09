#ifndef __BIRD_AI_HPP__
#define __BIRD_AI_HPP__

#include <chrono>
#include <vector>

#include "PhysicsAnalysis.hpp"
#include "Exceptions.hpp"
#include "Rectangle.hpp"
#include "PeriodicRunner.hpp"

class PhysicsAnalysis;
class ScreenIO;
class VideoFrame;

namespace Exceptions {

class AIException : public Exception
{
public:
	AIException(const std::string& exceptionMessage,
	            const std::string& callingFunctionName)
		: Exception(exceptionMessage, callingFunctionName, "AI")
	{ }

	virtual ~AIException() noexcept { }
};

}


class BirdAI {

public:

	struct StatusPacket {
		StatusPacket(const Rectangle& gr, const Rectangle& b, const std::vector<Rectangle>& obs) :
			gameRect(gr), bird(b), obstacles(obs)
		{ }

		Rectangle gameRect;
		Rectangle bird;
		std::vector<Rectangle> obstacles;
	};

	BirdAI(PhysicsAnalysis& phys, ScreenIO* sio) :
		currentState(AS_LAUNCH), physics(phys), io(sio), clicker(200)
	{ }

	void iterate(StatusPacket& pack, VideoFrame& frame);

private:

	typedef std::chrono::high_resolution_clock Clock;
	typedef std::chrono::duration<float, std::chrono::seconds::period> FloatingSeconds;

	enum State {
		AS_LAUNCH,
		AS_FALLING, ///< Drop to our starting point (cruising altitude)
		AS_HOW_HIGH, ///< Determine jump characteristics
		AS_GAUNTLET, ///< Let's do this
		AS_WAIT_FOR_LIFTOFF ///< Special state: waiting to go up
	};

	void updateState(StatusPacket& pack, VideoFrame& frame);

	void launch();

	void fall();

	void howHigh();

	void gauntlet();

	void waitForLiftoff();

	/// Send a click and wait for liftoff
	void fireRockets();

	State currentState;
	PhysicsAnalysis& physics;
	ScreenIO* io;

	State returnToState;

	int jumpHeight; ///< How high we can jump
	FloatingSeconds jumpDuration; ///< How long it takes us to jump that high
	int jumpWidth = -1; ///< The width of a the upward arc of a jump

	int cruisingAltitude; ///< Where to start for jump runs

	// Used for calculating jump height and duration
	std::vector<int> jumpHeights;
	std::vector<FloatingSeconds> jumpDurations;

	float currentVelocity; ///< Current vertical velocity of the bird
	float lastVelocity; ///< Previous vertical velocity of the bird

	// If some of these make no sense, look at updateState

	int birdLowestRadius = 0; ///< The lowest point of the bird from its center
	int birdHighestRadius = 0; ///< The highest point of the bird from its center
	int birdFarthestLeadingEdge = 0; ///< The farthest forward the bird is from its center

	Rectangle bird; ///< The bird's rectangle
	int floorY; ///< The floor's Y coordinate
	int closestObstaclesLeft; ///< The left bound of the nearest obstacle
	int closestObstaclesRight; ///< The right bound of the nearest obstacle
	int gapTop; ///< The top of the gap through which we must pass
	int gapBottom; ///< The bottom of the gap through which we must pass

	Clock::time_point jumpTimerStart; ///< Used for determining speeds, distances, etc.
	Clock::time_point pipeTimerStart; ///< Used for determining jumpWidth
	PeriodicRunner<std::chrono::milliseconds> clicker;
};

#endif
