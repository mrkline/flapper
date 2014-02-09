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

	std::shared_ptr<VideoFrame> getFrame() override;

	/// Focuses in on a certain part of the screen. Future calls to getFrame will just get this portion.
	void focusOn(const Rectangle& r) override;

	void resetFocus() override;

	void mouseTo(int x, int y);

	void click();

	// No copy or assign
	X11ScreenIO(const X11ScreenIO&) = delete;
	X11ScreenIO& operator=(const X11ScreenIO&) = delete;

private:

	Display* mainDisplay;
	Window rootWindow;
	unsigned int screenWidth, screenHeight;
	Rectangle capRect;
};

#endif
