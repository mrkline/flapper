#include "X11ScreenIO.hpp"

#include <cstdint>

#include "Exceptions.hpp"

using namespace std;

X11ScreenIO::X11ScreenIO()
{
	mainDisplay = XOpenDisplay(NULL);
	if (!mainDisplay)
		throw Exceptions::IOException("Could not open the default X11 display", __FUNCTION__);

	rootWindow = DefaultRootWindow(mainDisplay);
	resetFocus();
	// Throwaway values
	Window root;
	int x, y;
	unsigned int borderWidth, depth;

	if (!XGetGeometry(mainDisplay, rootWindow, &root,
	                  &x, &y, &screenWidth, &screenHeight, &borderWidth, &depth)) {
		throw Exceptions::IOException("Couldn't get geometry of the default X11 display", __FUNCTION__);
	}
	if (depth != 24) {
		throw Exceptions::IOException("This program assumes a 24-bit display."
		                              " This does not seem to be the case.", __FUNCTION__);
	}
	resetFocus();
}

X11ScreenIO::~X11ScreenIO()
{
	XCloseDisplay(mainDisplay);
}

std::shared_ptr<VideoFrame> X11ScreenIO::getFrame()
{
	// Still skewing oddly under circumstances. For now, avoid those. Later, figure out why.

	XImage* img = XGetImage(mainDisplay, rootWindow, 0, 0, screenWidth, screenHeight, AllPlanes, ZPixmap);
	if (img->depth != 24) {
		throw Exceptions::IOException("This program assumes a 24-bit display."
		                              " This does not seem to be the case.", __FUNCTION__);
	}
	if (img->bitmap_pad != 32) {
		throw Exceptions::IOException("This program assumes 32-bit padded pixels from X11."
		                              "This does not seem to be the case.", __FUNCTION__);
	}

	std::shared_ptr<VideoFrame> ret = make_shared<VideoFrame>(capRect.getWidth(), capRect.getHeight(), 3, false);

	auto curr = ret->getPixels();

	for (int y = 0; y < img->height; ++y) {
		uint32_t* line_ptr = (uint32_t*) &(img->data)[y * img->bytes_per_line];
		for (int x = 0; x < img->width; ++x) {
			if (!capRect.contains(x, y))
				continue;

			uint32_t pixelvalue = line_ptr[x];
			curr[0] = (uint8_t)((pixelvalue & 0x00FF0000) >> 16);
			curr[1] = (uint8_t)((pixelvalue & 0x0000FF00) >> 8);
			curr[2] = (uint8_t)((pixelvalue & 0x000000FF));
			curr += 3;
		}
	}
	XDestroyImage(img);
	return ret;
}

void X11ScreenIO::focusOn(const Rectangle& r)
{
	if (r.left >= r.right || r.top >= r.bottom || r.left < 0 || r.top < 0)
		throw Exceptions::ArgumentException("Invalid bounds", __FUNCTION__);

	capRect = r;
}

void X11ScreenIO::resetFocus()
{
	capRect.left = 0;
	capRect.top = 0;
	capRect.right = screenWidth - 1;
	capRect.bottom = screenHeight - 1;
}
