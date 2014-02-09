#ifndef __SCREEN_IO_HPP__
#define __SCREEN_IO_HPP__

#include <memory>

#include "VideoFrame.hpp"
#include "Rectangle.hpp"

/// Interface for screen grabbing and clicks
class ScreenIO {

public:

	/// Gets a frame from the screen
	virtual std::shared_ptr<VideoFrame> getFrame() = 0;

	/// Focuses in on a certain part of the screen. Future calls to getFrame will just get this portion.
	virtual void focusOn(const Rectangle& r) = 0;

	/// Undoes any focusing we've done and takes frames of the entire screen again
	virtual void resetFocus() = 0;

	/// Moves the mouse to a given position
	virtual void mouseTo(int x, int y) = 0;

	/// clicks
	virtual void click() = 0;

};

#endif
