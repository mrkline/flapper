#include "DisplayWindow.hpp"
#include "ui_DisplayWindow.h"

#include <QVBoxLayout>
#include <QPushButton>

#include <mutex>
#include <cstdio> // TEMP

#include "QGLCanvas.hpp"
#include "X11ScreenIO.hpp"
#include "FlappySearches.hpp"

using namespace std;

DisplayWindow::DisplayWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::DisplayWindow),
	canvas(new QGLCanvas),
	btnStart(new QPushButton("Start")),
	screenIO(new X11ScreenIO)
{
	threadRunning = false; // Probably redundant

	ui->setupUi(this);

	// Set up a layout containing our canvas
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(canvas);
	layout->addWidget(btnStart);
	ui->centralWidget->setLayout(layout);

	connect(btnStart, &QPushButton::clicked, this, &DisplayWindow::startClicked);

	canvas->setFrame(new QImage("placeholder.png"));
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
		return;
	}

	printf("Game window found! left: %d, top: %d, right: %d, bottom: %d\n",
	       gameRect.left, gameRect.top, gameRect.right, gameRect.bottom);
	fflush(stdout);

	screenIO->focusOn(gameRect);

	auto gameCap = screenIO->getFrame();


	QImage gameImage(gameCap->getPixels(),
	                 gameCap->getWidth(),
	                 gameCap->getHeight(),
	                 QImage::Format_RGB888);
	gameImage.save("game_test.png");

	canvas->setFrame(gameCap);

	// While we're not told to exit and there are more frames to display
	while (threadRunning) {

		// std::this_thread::sleep_for(sc::milliseconds(1000));
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
