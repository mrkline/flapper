#include "BufferedFrameFetcher.hpp"

#include "ScreenIO.hpp"

using namespace std;

BufferedFrameFetcher::BufferedFrameFetcher(ScreenIO* sio) : io(sio)

{
	threadRunning = true;
	worker.reset(new std::thread(&BufferedFrameFetcher::workerProc, this));
}

BufferedFrameFetcher::~BufferedFrameFetcher()
{
	threadRunning = false;
	worker->join();
}

std::shared_ptr<VideoFrame> BufferedFrameFetcher::getFrame()
{
	unique_lock<mutex> ml(frameLock);
	frameCV.wait(ml, [this] { return frameReady; });
	return frame;
}


void BufferedFrameFetcher::workerProc()
{
	while (threadRunning) {

		auto newFrame = io->getFrame();

		// Only lock on the assignment so we don't hold the lock while we actually get the frame
		{
			lock_guard<mutex> ml(frameLock);
			frame = newFrame;
			frameReady = true;
			frameCV.notify_one();
		}
		tracker.onFrame();
	}
}
