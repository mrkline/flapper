#ifndef __BIRD_AI_HPP__
#define __BIRD_AI_HPP__

#include <chrono>
#include <vector>

#include "PhysicsAnalysis.hpp"
#include "Exceptions.hpp"
#include "Rectangle.hpp"

class PhysicsAnalysis;
class ScreenIO;

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

	BirdAI(PhysicsAnalysis& phys, ScreenIO* sio) : currentState(AS_LAUNCH), physics(phys), io(sio) { }

	void iterate(StatusPacket& pack);

private:

	typedef std::chrono::high_resolution_clock Clock;
	typedef std::chrono::duration<float, std::chrono::seconds::period> FloatingSeconds;

	enum State {
		AS_LAUNCH,
		AS_HOW_HIGH, ///< Determine jump characteristics
		AS_GAUNTLET, ///< Let's do this
		AS_WAIT_FOR_LIFTOFF ///< Special state: waiting to go up
	};

	void updateState(StatusPacket& pack);

	void launch();

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
	int cruisingAltitude; ///< Where to start for jump runs

	std::vector<int> jumpRuns;

	// If some of these make no sense, look at updateState
	int birdY;
	int birdLowestRadius;
	int birdHighestRadius;
	int birdFarthestLeadingEdge;
	int floorY;
	int closestObstaclesLeft;
	int closestObstaclesRight;
	int gapTop;
	int gapBottom;

	Clock::time_point timerStart;
};

#endif
