#ifndef __BUFFERED_FRAME_FETCHER_HPP__
#define __BUFFERED_FRAME_FETCHER_HPP__

#include "VideoFrame.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "FPSTracker.hpp"

class ScreenIO;

// Fetches frames using a separate thread into a double buffer
class BufferedFrameFetcher final {

public:

	BufferedFrameFetcher(ScreenIO* sio);

	~BufferedFrameFetcher();

	std::shared_ptr<VideoFrame> getFrame();

	FPSTracker& getFPSTracker() { return tracker; }

	BufferedFrameFetcher(const BufferedFrameFetcher&) = delete;
	BufferedFrameFetcher& operator=(const BufferedFrameFetcher&) = delete;

private:

	void workerProc();

	// Lock when switching frames over, just to be safe.
	// This may be totally unneeded.
	std::mutex frameLock;
	std::condition_variable frameCV;
	bool frameReady = false;

	std::shared_ptr<VideoFrame> frame;

	std::unique_ptr<std::thread> worker;
	std::atomic<bool> threadRunning; ///< Set to true when the video updating thread should exit

	ScreenIO* io;
	FPSTracker tracker;
};

#endif
