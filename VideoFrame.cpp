#include "VideoFrame.hpp"

#include "MKMath.hpp"

void VideoFrame::rgb2hsv()
{
	using namespace Math;

	if (depth != 3)
		throw Exceptions::ArgumentException("The frame must be 24-bit RGB", __FUNCTION__);
	
	uint8_t* pix = pixels;
	uint8_t* end = pix + getTotalSize();

	while (pix < end) {
		float r = (float)pix[0] / 255.0f;
		float g = (float)pix[1] / 255.0f;
		float b = (float)pix[2] / 255.0f;

		float cmax = std::max({r, g, b});
		float cmin = std::min({r, g, b});
		float delta = cmax - cmin;

		float h;
		if (cmax == r)
			h = 60.0f * (g - b)/delta;
		else if (cmax == g)
			h = 60.0f * ((b - r)/delta + 2);
		else
			h = 60.0f * ((r -g)/delta + 4);

		while (h < 0.0f)
			h += 360.0f;
		while (h > 360.0f)
			h -= 360.0f;

		float s;
		// Should use ulps, but we'll live for now
		if (isZero(delta))
			s = 0;
		else
			s = delta / cmax;

		float v = cmax;

		pix[0] = (uint8_t)(h / 360.0f * 255.0f);
		pix[1] = (uint8_t)(s * 255.0f);
		pix[2] = (uint8_t)(v * 255.0f);
		pix += 3;
	}
}

void VideoFrame::crosshairsAt(Point p, uint8_t color[3], int radius)
{

	if (depth != 3)
		throw Exceptions::ArgumentException("The frame must be 24-bit RGB", __FUNCTION__);

	if (p.x < 0 || p.x >= (int)width || p.y < 0 || p.y >= (int)height)
		throw Exceptions::ArgumentException("Invalid point", __FUNCTION__);

	const size_t left = (size_t)std::max(p.x - radius, 0);
	const size_t top = (size_t)std::max(p.y - radius, 0);
	const size_t right = (size_t)std::min(p.x + radius, (int)width - 1);
	const size_t bottom = (size_t)std::min(p.y + radius, (int)height - 1);

	for (size_t y = top; y < (size_t)p.y; ++y) {
		auto pix = getPixel((size_t)p.x, y);
		pix[0] = color[0];
		pix[1] = color[1];
		pix[2] = color[2];
	}

	for (size_t x = left; x <= right; ++x) {
		auto pix = getPixel(x, (size_t)p.y);
		pix[0] = color[0];
		pix[1] = color[1];
		pix[2] = color[2];
	}

	for (size_t y = p.y + 1; y <= bottom; ++y) {
		auto pix = getPixel((size_t)p.x, y);
		pix[0] = color[0];
		pix[1] = color[1];
		pix[2] = color[2];
	}
}
