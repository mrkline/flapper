#include "FlappySearches.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include "VideoFrame.hpp"

using namespace std;

namespace {

const array<uint8_t, 3> flappySkyRGB = { 112, 198, 206 };
const array<uint8_t, 3> flappyGroundRGB = { 221, 218, 147 };
const array<uint8_t, 3> beakRGB = { 244, 106, 78 };
const array<uint8_t, 3> gameOverRGB = { 255, 255, 255 };

const vector<array<uint8_t, 3>> birdRGBs = { beakRGB, {252, 239, 40}, {249, 187, 4} };
const vector<array<uint8_t, 3>> pipeRGBs = { {205, 252, 113}, {139, 230, 68}, {96, 182, 34}, {66, 121, 25} };

const float normalizedBirdSize = 62.0f / 500.0f; // Size of the bird relative to the screen's width

inline bool pixelIsApprox(const uint8_t* pix, array<uint8_t, 3> to, int tolerance = 5)
{
	const int r = pix[0];
	const int g = pix[1];
	const int b = pix[2];

	const int tr = to[0];
	const int tg = to[1];
	const int tb = to[2];

	return abs(r - tr) <= tolerance && abs(g - tg) <= tolerance && abs(b - tb) <= tolerance;
}

void mergeAdjacentRects(vector<Rectangle>& rectList)
{
	if (rectList.size() <= 1)
		return;

	bool mergedOne;
	do {
		mergedOne = false;

		const Rectangle& last = rectList.back();
		for (int i = (int)rectList.size() - 2; i >= 0; --i) {
			if (rectList[i].adjacentTo(last)) {
				rectList[i].expandTo(last);
				rectList.pop_back(); // erase last
				mergedOne = true;
				break;
			}
		}
	} while (mergedOne);
}

auto biggestRect = [](const Rectangle& l, const Rectangle& r) { return l.getArea() > r.getArea(); };

} // end anonymous namespace

Rectangle findGameWindow(const VideoFrame& frame)
{
	vector<Rectangle> skyRects;
	vector<Rectangle> groundRects;

	frame.foreachPixel([&](const uint8_t* pix, int x, int y) {

		auto adjacent = [=](const Rectangle& r) { return r.adjacentTo(x, y); };

		if (pixelIsApprox(pix, flappySkyRGB)) {
			auto inside = find_if(begin(skyRects), end(skyRects), adjacent);
			if (inside != end(skyRects))
				inside->expandTo(x, y);
			else
				skyRects.emplace_back(x, y, x, y);
		}
		else if (pixelIsApprox(pix, flappyGroundRGB)) {
			auto inside = find_if(begin(groundRects), end(groundRects), adjacent);
			if (inside != end(groundRects))
				inside->expandTo(x, y);
			else
				groundRects.emplace_back(x, y, x, y);
		}

		return true;
	});

	if (skyRects.empty())
		throw Exceptions::Exception("Could not find a single sky rectangle", __FUNCTION__);

	if (groundRects.empty())
		throw Exceptions::Exception("Could not find a single ground rectangle", __FUNCTION__);

	mergeAdjacentRects(skyRects);
	mergeAdjacentRects(groundRects);

	sort(begin(skyRects), end(skyRects), biggestRect);
	sort(begin(groundRects), end(groundRects), biggestRect);

	const Rectangle& bigSky = skyRects[0];
	const Rectangle& bigGround = groundRects[0];

	if (bigSky.left != bigGround.left || bigSky.right != bigGround.right)
		throw Exceptions::Exception("The sky and ground rectangles do not line up", __FUNCTION__);

	return Rectangle(bigSky.left, bigSky.top, bigGround.right, bigGround.bottom);
}

Point findBeakLocation(const VideoFrame& frame)
{
	vector<Rectangle> beakRects;

	frame.foreachPixel([&](const uint8_t* pix, int x, int y) {

		auto adjacent = [=](const Rectangle& r) { return r.adjacentTo(x, y); };

		if (pixelIsApprox(pix, beakRGB, 20)) {
			auto inside = find_if(begin(beakRects), end(beakRects), adjacent);
			if (inside != end(beakRects))
				inside->expandTo(x, y);
			else
				beakRects.emplace_back(x, y, x, y);
		}

		return true;
	});

	if (beakRects.empty())
		throw Exceptions::Exception("Could not find a single beak rectangle", __FUNCTION__);

	mergeAdjacentRects(beakRects);

	sort(begin(beakRects), end(beakRects), biggestRect);

	return beakRects[0].getCenter();
}

Rectangle findBird(const VideoFrame& frame, const Point beak)
{
	Rectangle within(beak);
	within.expandBy((int)(normalizedBirdSize * (float)frame.getWidth()));
	within.constrainBy(Rectangle(0, 0, (int)frame.getWidth() - 1, (int)frame.getHeight() - 1));

	Rectangle bird(beak);

	for (int y = within.top; y <= within.bottom; ++y) {
		const uint8_t* pixel = frame.getPixel((size_t)within.left, (size_t)y);
		for (int x = within.left; x <= within.right; ++x, pixel +=3) {
			if (any_of(begin(birdRGBs), end(birdRGBs),
			        [=](const array<uint8_t, 3>& color) { return pixelIsApprox(pixel, color, 20); })) {
				bird.expandTo(x, y);
			}
		}
	}

	return bird;
}

vector<Rectangle> findPipes(const VideoFrame& frame)
{
	vector<Rectangle> pipes;

	frame.foreachPixel([&](const uint8_t* pix, int x, int y) {

		auto adjacent = [=](const Rectangle& r) { return r.adjacentTo(x, y, 5); };

		if (any_of(begin(pipeRGBs), end(pipeRGBs),
				[=](const array<uint8_t, 3>& color) { return pixelIsApprox(pix, color, 20); })) {
			auto inside = find_if(begin(pipes), end(pipes), adjacent);
			if (inside != end(pipes))
				inside->expandTo(x, y);
			else
				pipes.emplace_back(x, y, x, y);
		}

		return true;
	});

	mergeAdjacentRects(pipes);

	return pipes;
}

bool gameOver(const VideoFrame& frame)
{
	// The screen flashes white when the game ends

	bool over = true;
	frame.foreachPixel([&](const uint8_t* pix, int x, int y) {
		(void)x; // Shut up, compiler
		(void)y; // Shut up, compiler
		if (!pixelIsApprox(pix, gameOverRGB)) {
			over = false;
			return false;
		}
		else return true;
	});

	return over;
}
