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
#include "FPSTracker.hpp"
#include "PeriodicRunner.hpp"
#include "PhysicsAnalysis.hpp"
#include "BirdAI.hpp"

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

	Rectangle gameRect;
	try {
		gameRect = findGameWindow(*fullscreenFrame);
	}
	catch(const Exceptions::Exception& e) {
		fprintf(stderr, "Could not find game window with error:\n");
		fprintf(stderr, "%s\nin function %s\n", e.message.c_str(), e.callingFunction.c_str());
		fflush(stderr);
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

	FPSTracker processingTracker;
	FPSTracker failureTracker;

	PhysicsAnalysis physics(10);
	PeriodicRunner<std::chrono::milliseconds> physicsPrinter(50);

	BirdAI ai(physics, screenIO.get());

	screenIO->mouseTo(gameRect.getCenter());
	for (int i = 0; i < 10; ++i) screenIO->click();

	// While we're not told to exit and there are more frames to display
	while (threadRunning) {
		auto currentFrame = fetcher.getFrame();

		try {
			if (gameOver(*currentFrame)) {
				printf("Game over!");
				fflush(stdout);
				break;
			}

			Point beakLocation = findBeakLocation(*currentFrame);
			Rectangle bird = findBird(*currentFrame, beakLocation);
			bird.expandBy(5); // Give ourselves some padding
			auto pipes = findPipes(*currentFrame);

			physics.logPosition(bird.getCenter().y);

			BirdAI::StatusPacket statusPack(gameRect, bird, pipes);

			std::array<uint8_t, 3> crosshairColor = { 170, 40, 252 };
			std::array<uint8_t, 3> birdOverlayColor = { 170, 40, 252 };
			currentFrame->rectangleAt(bird, birdOverlayColor);
			currentFrame->crosshairsAt(beakLocation, crosshairColor, 30);

			ai.iterate(statusPack, *currentFrame);

			/*
			if (physics.hasAcceleration()) {
				physicsPrinter.runPeriodically([&physics]() {
					printf("Physics: P: %4.3f, V: %4.3f, A: %4.3f\n",
					       physics.getAveragePosition(),
					       physics.getAverageVelocity(),
					       physics.getAverageAcceleration());
					fflush(stdout);
				});
			}
			*/

			processingTracker.onFrame();
		}
		catch(const Exceptions::IOException& e) {
			fprintf(stderr, "IO problem!\n%s in %s\n", e.message.c_str(), e.callingFunction.c_str());
			return;
		}
		catch(const Exceptions::Exception& e) {
			failureTracker.onFrame();
			/*
			static int num = 1;
			QImage errorImg(currentFrame->getPixels(),
							currentFrame->getWidth(),
							currentFrame->getHeight(),
							QImage::Format_RGB888);
			QString fn = "wat";
			fn += QString::number(num++);
			fn += ".png";
			errorImg.save(fn);
			*/
		}

		canvas->setFrame(currentFrame);
		fetcher.getFPSTracker().printPeriodically("Recording FPS: ");
		processingTracker.printPeriodically("Processing FPS: ");
		failureTracker.printPeriodically("Failures/second: ");
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
