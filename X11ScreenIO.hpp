#ifndef __X11_SCREEN_IO_HPP__
#define __X11_SCREEN_IO_HPP__

#include "ScreenIO.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

/// An X11 implementation of ScreenIO
class X11ScreenIO : public ScreenIO {

public:

	X11ScreenIO();

	~X11ScreenIO();

	std::unique_ptr<VideoFrame> getFrame() override;

	/// Focuses in on a certain part of the screen. Future calls to getFrame will just get this portion.
	void focusOn(int left, int top, int right, int bottom) override;

	// No copy or assign
	X11ScreenIO(const X11ScreenIO&) = delete;
	X11ScreenIO& operator=(const X11ScreenIO&) = delete;

private:

	Display* mainDisplay;
	Window rootWindow;
	int capX, capY;
	unsigned int capWidth, capHeight;
};

#endif
