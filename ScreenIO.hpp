#ifndef __SCREEN_IO_HPP__
#define __SCREEN_IO_HPP__

#include <memory>

#include "VideoFrame.hpp"

/// Interface for screen grabbing and clicks
class ScreenIO {

public:

	/// Gets a frame from the screen
	virtual std::unique_ptr<VideoFrame> getFrame() = 0;

	/// Focuses in on a certain part of the screen. Future calls to getFrame will just get this portion.
	virtual void focusOn(int left, int top, int right, int bottom) = 0;
};

#endif
