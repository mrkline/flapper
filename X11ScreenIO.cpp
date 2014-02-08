#include "X11ScreenIO.hpp"

#include "Exceptions.hpp"

X11ScreenIO::X11ScreenIO()
{
	mainDisplay = XOpenDisplay(NULL);
	if (!mainDisplay)
		throw Exceptions::IOException("Could not open the default X11 display", __FUNCTION__);

	rootWindow = DefaultRootWindow(mainDisplay);

	// Throwaway values
	Window root;
	unsigned int borderWidth, depth;

	if (!XGetGeometry(mainDisplay, rootWindow, &root,
	                  &capX, &capY, &capWidth, &capHeight, &borderWidth, &depth)) {
		throw Exceptions::IOException("Couldn't get geometry of the default X11 display", __FUNCTION__);
	}
	if (depth != 24) {
		throw Exceptions::IOException("This program assumes a 24-bit display."
		                              " This does not seem to be the case.", __FUNCTION__);
	}
}

std::unique_ptr<VideoFrame> X11ScreenIO::getFrame()
{
	return std::unique_ptr<VideoFrame>();
}

void X11ScreenIO::focusOn(int left, int top, int right, int bottom)
{
}
