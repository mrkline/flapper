#ifndef __FLAPPY_SEARCHES_HPP__
#define __FLAPPY_SEARCHES_HPP__

/**
 * \file FlappySearches.hpp
 *
 * Finds things in Flappy Bird. Could be replaced with machine learning stuff if I have the time.
 */

#include <vector>

#include "Rectangle.hpp"

class VideoFrame;

Rectangle findGameWindow(const VideoFrame& frame);

Point findBeakLocation(const VideoFrame& frame);

Rectangle findBird(const VideoFrame& frame, const Point beak);

std::vector<Rectangle> findPipes(const VideoFrame& frame);

bool gameOver(const VideoFrame& frame);

#endif
