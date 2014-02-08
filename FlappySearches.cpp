#include "FlappySearches.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "VideoFrame.hpp"

using namespace std;

namespace {

uint8_t flappySkyRGB[3] = { 112, 198, 206 };
uint8_t flappyGroundRGB[3] = { 221, 218, 147 };
uint8_t beakRGB[3] = { 244, 106, 78 };

// std::vector<uint8_t[3]> = { beakRGB,

inline bool pixelIsApprox(const uint8_t* pix, uint8_t to[3], int tolerance = 5)
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

		const Rectangle& last = rectList[rectList.size() - 1];
		for (int i = (int)rectList.size() - 2; i >= 0; --i) {
			if (rectList[i].adjacentTo(last)) {
				rectList[i].expandTo(last);
				rectList.erase(begin(rectList) + (rectList.size() - 1)); // erase last
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
