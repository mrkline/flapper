#include "DisplayWindow.hpp"
#include "ui_DisplayWindow.h"

#include <QVBoxLayout>
#include <QPushButton>

#include <mutex>
#include <cstdio> // TEMP

#include "QGLCanvas.hpp"
#include "X11ScreenIO.hpp"
#include "FlappySearches.hpp"
#include "BufferedFrameFetcher.hpp"

using namespace std;

DisplayWindow::DisplayWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::DisplayWindow),
	canvas(new QGLCanvas),
	btnStart(new QPushButton("Start")),
	threadRunning(false),
	screenIO(new X11ScreenIO)
{
	ui->setupUi(this);

	// Set up a layout containing our canvas
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(canvas);
	layout->addWidget(btnStart);
	ui->centralWidget->setLayout(layout);

	connect(btnStart, &QPushButton::clicked, this, &DisplayWindow::startClicked);

	canvas->setFrame(unique_ptr<QImage>(new QImage("StartImage.jpg")));
}

DisplayWindow::~DisplayWindow()
{
	if (threadRunning) {
		threadRunning = false;
		playThread->join();
	}
	delete ui;
}

void DisplayWindow::play()
{
	namespace sc = std::chrono;

	// First let's find the window. It's going to have a bunch of blue up top and some tan down below
	screenIO->resetFocus();
	auto fullscreenFrame = screenIO->getFrame();
	auto hsvFullscreen(*fullscreenFrame);
	hsvFullscreen.rgb2hsv();

	QImage hsvImage(hsvFullscreen.getPixels(),
	                hsvFullscreen.getWidth(),
	                hsvFullscreen.getHeight(),
	                QImage::Format_RGB888);
	hsvImage.save("hsv_test.png");

	Rectangle gameRect;
	try {
		gameRect = findGameWindow(hsvFullscreen);
	}
	catch(const Exceptions::Exception& e) {
		fprintf(stderr, "Could not find game window with error:\n");
		fprintf(stderr, "%s\nin function %s\n", e.message.c_str(), e.callingFunction.c_str());
		canvas->setFrame(fullscreenFrame);
		this_thread::sleep_for(sc::seconds(5));
		canvas->setFrame(unique_ptr<QImage>(new QImage("ErrorImage.jpg")));
		return;
	}

	printf("Game window found! left: %d, top: %d, right: %d, bottom: %d\n",
	       gameRect.left, gameRect.top, gameRect.right, gameRect.bottom);
	fflush(stdout);

	screenIO->focusOn(gameRect);

	auto gameCap = screenIO->getFrame();

	// canvas->setFrame(fullscreenFrame);
	// this_thread::sleep_for(sc::seconds(5));
	canvas->setFrame(gameCap);

	BufferedFrameFetcher fetcher(screenIO.get());

	// While we're not told to exit and there are more frames to display
	while (threadRunning) {
		auto currentFrame = fetcher.getFrame();
		unique_ptr<QImage> loopTest(new QImage(currentFrame->getPixels(),
		                currentFrame->getWidth(),
		                currentFrame->getHeight(),
		                QImage::Format_RGB888));
		canvas->setFrame(std::move(loopTest));
		// canvas->setFrame(fetcher.getFrame());
		// fetcher.getFPSTracker().printPeriodically("Fetcher FPS: ");
		// fflush(stdout);
		// canvas->setFrame(nextMask);
		// Post a new order to repaint. Done this way because another thread cannot directly call repaint()
		//QCoreApplication::postEvent(canvas, new QPaintEvent(canvas->rect()));
	}
}

void DisplayWindow::startClicked()
{
	if (threadRunning) {
		threadRunning = false;
		playThread->join();
	}
	threadRunning = true;
	playThread.reset(new std::thread(&DisplayWindow::play, this));
}
