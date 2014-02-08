#include <QCoreApplication>

#include "QGLCanvas.hpp"

using namespace std;

QGLCanvas::QGLCanvas(QWidget* parent)
	: QGLWidget(parent)
{
}

void QGLCanvas::setFrame(const std::shared_ptr<VideoFrame>& newFrame)
{
	// Keep a shared_ptr reference to our frame data
	frame = newFrame;

	// Create a new QImage, which is just a shallow copy of the frame.
	setFrame(new QImage(frame->getPixels(),
	                    frame->getWidth(),
	                    frame->getHeight(),
	                    QImage::Format_RGB888));
}

void QGLCanvas::setFrame(QImage* image)
{
	// Keep the QGL canvas from drawing while we change the image
	lock_guard<mutex> pixelLock(pixelsMutex);

	img.reset(image);
	QCoreApplication::postEvent(this, new QPaintEvent(rect()));
}

void QGLCanvas::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, 1);

	// Lock the image so that other threads can't access it at the same time
	lock_guard<mutex> pixelLock(pixelsMutex);
	painter.drawImage(this->rect(), *img);
}
