#include "BirdAI.hpp"

#include "ScreenIO.hpp"

#include <sstream>

using namespace std;

namespace {

auto highestRect = [](const Rectangle& l, const Rectangle& r) { return l.bottom < r.bottom; };
auto leftMostRect = [](const Rectangle& l, const Rectangle& r) { return l.left < r.left; };
auto filterSmall = [](const Rectangle& r) { return r.getArea() < 20; };

} // end anonymous namespace

void BirdAI::iterate(StatusPacket& pack)
{
	updateState(pack);

	switch (currentState) {
		case AS_LAUNCH:
			launch();
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

void BirdAI::updateState(StatusPacket& pack)
{
	const int close = 5;

	// All other coordinates are relative to this guy.
	pack.gameRect.right -= pack.gameRect.left;
	pack.gameRect.bottom -= pack.gameRect.top;
	pack.gameRect.left = 0;
	pack.gameRect.top = 0;

	cruisingAltitude = std::min(pack.gameRect.bottom - 100, pack.gameRect.getCenter().y + 100);

	birdY = pack.bird.getCenter().y;
	birdLowestRadius = std::max(birdLowestRadius, pack.bird.bottom - birdY);
	birdHighestRadius = std::max(birdHighestRadius, birdY - pack.bird.top);
	birdFarthestLeadingEdge = std::max(birdFarthestLeadingEdge, pack.bird.right);

	auto& obstacles = pack.obstacles;

	obstacles.erase(remove_if(begin(obstacles), end(obstacles), filterSmall), end(obstacles));

	sort(begin(obstacles), end(obstacles), highestRect);

	/*
	printf("Obstacles:\n");
	for (const auto& r : obstacles) {
		printf("\t(%d,%d; %d,%d)\n", r.left, r.top, r.right, r.bottom);
	}
	fflush(stdout);
	*/
	
	auto& floor = obstacles.back();

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

	gapTop = top->bottom;
	gapBottom = bottom->top;
	closestObstaclesLeft = std::min(top->left, bottom->left);
	closestObstaclesRight = std::max(top->right, bottom->right);
}

void BirdAI::launch()
{
	currentState = AS_HOW_HIGH;
	fireRockets();
	printf("AI: Launch sequence initiated\n");
	fflush(stdout);
}

void BirdAI::howHigh()
{
	if (physics.getAverageVelocity() >= 0) {
		if (birdY >= cruisingAltitude) {
			printf("AI: Starting jump %d", (int)jumpRuns.size() + 1);
			jumpHeight = birdY;
			fireRockets();
		}
		else {
			jumpRuns.emplace_back(jumpHeight - birdY);
			printf("AI: Jump test %d: %d pixels\n", (int)jumpRuns.size(), jumpRuns.back());
			if (jumpRuns.size() == 3) {
				jumpHeight = 0;
				for (int run : jumpRuns)
					jumpHeight += run;
				jumpHeight /= jumpRuns.size();
				printf("AI: Averaged jump height is %d. Beginning run\n", jumpHeight);
				currentState = AS_GAUNTLET;
			}
		}
	}
	fflush(stdout);
}

void BirdAI::gauntlet()
{
}

void BirdAI::waitForLiftoff()
{
	// Positive means we're going down
	if (physics.getAverageVelocity() < 0) {
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
}
