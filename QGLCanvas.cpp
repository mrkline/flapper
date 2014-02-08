#include <QCoreApplication>

#include "QGLCanvas.hpp"

using namespace std;

QGLCanvas::QGLCanvas(QWidget* parent)
	: QGLWidget(parent), paintMessageSent(false)
{
}

void QGLCanvas::setFrame(const std::shared_ptr<VideoFrame>& newFrame)
{
	// Keep the QGL canvas from drawing while we change the image
	lock_guard<recursive_mutex> pixelLock(pixelsMutex);

	// Keep a shared_ptr reference to our frame data
	frame = newFrame;

	// Create a new QImage, which is just a shallow copy of the frame.
	setFrame(std::unique_ptr<QImage>(new QImage(frame->getPixels(),
	                                 frame->getWidth(),
	                                 frame->getHeight(),
	                                 QImage::Format_RGB888)));
}

void QGLCanvas::setFrame(std::unique_ptr<QImage>&& image)
{
	lock_guard<recursive_mutex> pixelLock(pixelsMutex);
	img = std::move(image);
	if (!paintMessageSent) {
		QCoreApplication::postEvent(this, new QPaintEvent(rect()));
		paintMessageSent = true;
	}
}

void QGLCanvas::paintEvent(QPaintEvent*)
{
	if (img == nullptr)
		return;

	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, 1);

	// Lock the image so that other threads can't access it at the same time
	lock_guard<recursive_mutex> pixelLock(pixelsMutex);
	painter.drawImage(this->rect(), *img);
	paintMessageSent = false;
}
