#ifndef __FLAPPY_SEARCHES_HPP__
#define __FLAPPY_SEARCHES_HPP__

/**
 * \file FlappySearches.hpp
 *
 * Finds things in Flappy Bird. Could be replaced with machine learning stuff if I have the time.
 */

#include "Rectangle.hpp"

class VideoFrame;

Rectangle findGameWindow(const VideoFrame& frame);

Point findBeakLocation(const VideoFrame& frame);

#endif
