#include "BirdAI.hpp"

#include "ScreenIO.hpp"
#include "VideoFrame.hpp"

#include <sstream>

using namespace std;

namespace {

auto highestRect = [](const Rectangle& l, const Rectangle& r) { return l.bottom < r.bottom; };
auto leftMostRect = [](const Rectangle& l, const Rectangle& r) { return l.left < r.left; };
auto filterSmall = [](const Rectangle& r) { return r.getArea() < 20; };

} // end anonymous namespace

void BirdAI::iterate(StatusPacket& pack, VideoFrame& frame)
{
	updateState(pack, frame);

	switch (currentState) {
		case AS_LAUNCH:
			launch();
			break;
		case AS_FALLING:
			fall();
			break;
		case AS_HOW_HIGH:
			howHigh();
			break;
		case AS_GAUNTLET:
			gauntlet();
			break;
		case AS_WAIT_FOR_LIFTOFF:
			waitForLiftoff();
			break;
	}

}

void BirdAI::updateState(StatusPacket& pack, VideoFrame& frame)
{
	const int close = 5;
	const std::array<uint8_t, 3> obstacleOverlayColor = { 0, 0, 0 };

	lastVelocity = currentVelocity;
	currentVelocity = physics.getAverageVelocity();

	// All other coordinates are relative to this guy.
	pack.gameRect.right -= pack.gameRect.left;
	pack.gameRect.bottom -= pack.gameRect.top;
	pack.gameRect.left = 0;
	pack.gameRect.top = 0;

	cruisingAltitude = std::min(pack.gameRect.bottom - 100, pack.gameRect.getCenter().y + 50);

	birdY = pack.bird.getCenter().y;
	birdLowestRadius = std::max(birdLowestRadius, pack.bird.bottom - birdY);
	birdHighestRadius = std::max(birdHighestRadius, birdY - pack.bird.top);
	birdFarthestLeadingEdge = std::max(birdFarthestLeadingEdge, pack.bird.right);

	auto& obstacles = pack.obstacles;

	obstacles.erase(remove_if(begin(obstacles), end(obstacles), filterSmall), end(obstacles));

	sort(begin(obstacles), end(obstacles), highestRect);

	auto& floor = obstacles.back();

	frame.rectangleAt(floor, obstacleOverlayColor);

	if (std::abs(floor.left - pack.gameRect.left) > close || std::abs(floor.right - pack.gameRect.right) > close) {
		stringstream err;
		err << "Floor doesn't seem to span the screen. Is it really the floor?\n";
		err << "Floor: (";
		err << floor.left;
		err << ",";
		err << floor.top;
		err << "; ";
		err << floor.right;
		err << ",";
		err << floor.bottom;
		err << ")\n";
		err << "game rect: (";
		err << pack.gameRect.left;
		err << ",";
		err << pack.gameRect.top;
		err << "; ";
		err << pack.gameRect.right;
		err << ",";
		err << pack.gameRect.bottom;
		err << ")";
		throw Exceptions::AIException(err.str(), __FUNCTION__);
	}

	floorY = floor.top;

	obstacles.pop_back();

	// We don't care about obstacles we've passed
	obstacles.erase(remove_if(begin(obstacles), end(obstacles),
	                          [&](const Rectangle& o) { return o.right < pack.bird.left; }),
	                end(obstacles));

	if (obstacles.empty()) {
		closestObstaclesLeft = closestObstaclesRight = gapTop = gapBottom = -1;
		return;
	}

	if (obstacles.size() % 2 != 0)
		throw Exceptions::AIException("Pipes should come in pairs", __FUNCTION__);

	sort(begin(obstacles), end(obstacles), leftMostRect);

	if (std::abs(obstacles[0].left - obstacles[1].left) > close ||
	    std::abs(obstacles[0].right - obstacles[1].right) > close)
		throw Exceptions::AIException("Two closest pipes don't match up", __FUNCTION__);

	Rectangle* top;
	Rectangle* bottom;

	if (obstacles[0].bottom < obstacles[1].top) {
		top = &obstacles[0];
		bottom = &obstacles[1];
	}
	else {
		bottom = &obstacles[0];
		top = &obstacles[1];
	}

	frame.rectangleAt(*top, obstacleOverlayColor);
	frame.rectangleAt(*bottom, obstacleOverlayColor);

	gapTop = top->bottom;
	gapBottom = bottom->top;
	closestObstaclesLeft = std::min(top->left, bottom->left);
	closestObstaclesRight = std::max(top->right, bottom->right);
}

void BirdAI::launch()
{
	currentState = AS_FALLING;
	fireRockets();
	printf("AI: Launch sequence initiated\n");
	fflush(stdout);
}

void BirdAI::fall()
{
	if (currentVelocity >= 0) {
		printf("Dropping to begin jump tests\n");
		currentState = AS_HOW_HIGH;
	}
	fflush(stdout);
}

void BirdAI::howHigh()
{
	if (currentVelocity >= 0 && birdY >= cruisingAltitude) {
		printf("AI: Starting jump %d\n", (int)jumpRuns.size() + 1);
		jumpHeight = birdY;
		fireRockets();
	}
	else if (currentVelocity >= 0 && lastVelocity < 0) {
		jumpRuns.emplace_back(jumpHeight - birdY);
		printf("AI: Jump test %d: %d pixels\n", (int)jumpRuns.size(), jumpRuns.back());
		if (jumpRuns.size() == 5) {
			jumpHeight = 0;
			// Throw out the first
			for (size_t i = 1; i < jumpRuns.size(); ++i)
				jumpHeight += jumpRuns[i];
			jumpHeight /= (jumpRuns.size() - 1);
			printf("AI: Averaged jump height is %d. Beginning run\n", jumpHeight);
			currentState = AS_GAUNTLET;
		}
	}
	fflush(stdout);
}

void BirdAI::gauntlet()
{
	const int fireDelayCompensation = 40;

	int floor;
	if (gapBottom < 0) // If there is no gap top
		floor = cruisingAltitude;
	else
		floor = gapBottom;
	
	const int adjusted = birdY + birdLowestRadius + fireDelayCompensation;
	printf("y %d adj %d fl %d\n", birdY, adjusted, floor);
	if (currentVelocity >= 0 && adjusted >= floor)
		fireRockets();
}

void BirdAI::waitForLiftoff()
{
	// Positive means we're going down
	if (currentVelocity < 0) {
		currentState = returnToState;
		printf("AI: Liftoff\n");
		fflush(stdout);
	}
}

void BirdAI::fireRockets()
{
	io->click();
	returnToState = currentState;
	currentState = AS_WAIT_FOR_LIFTOFF;
	printf("[Rocket Intensifies]\n");
	fflush(stdout);
}
