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
