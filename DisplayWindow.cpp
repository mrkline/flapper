#include "DisplayWindow.hpp"
#include "ui_DisplayWindow.h"

#include <QVBoxLayout>
#include <QPushButton>

#include <mutex>
#include <cstdio> // TEMP

#include "QGLCanvas.hpp"
#include "X11ScreenIO.hpp"

using namespace std;

DisplayWindow::DisplayWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::DisplayWindow),
	canvas(new QGLCanvas),
	btnStart(new QPushButton("Start")),
	screenIO(new X11ScreenIO)
{
	ui->setupUi(this);

	// Set up a layout containing our canvas
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(canvas);
	layout->addWidget(btnStart);
	ui->centralWidget->setLayout(layout);

	connect(btnStart, &QPushButton::clicked, this, &DisplayWindow::startClicked);

	canvas->setFrame(new QImage("placeholder.png"));

	videoUpdateThread.reset(new std::thread(&DisplayWindow::videoUpdate, this));
}

DisplayWindow::~DisplayWindow()
{
	threadExit = true;
	videoUpdateThread->join();
	delete ui;
}

void DisplayWindow::videoUpdate()
{
	namespace sc = std::chrono;

	// While we're not told to exit and there are more frames to display
	while (!threadExit) {

		std::this_thread::sleep_for(sc::milliseconds(1000));
		// canvas->setFrame(nextMask);
		// Post a new order to repaint. Done this way because another thread cannot directly call repaint()
		//QCoreApplication::postEvent(canvas, new QPaintEvent(canvas->rect()));
	}
}

void DisplayWindow::startClicked()
{
	auto frame = screenIO->getFrame();
	QImage img(frame->getPixels(),
	           frame->getWidth(),
	           frame->getHeight(),
	           QImage::Format_RGB888);
	img.save("out.png");
}
