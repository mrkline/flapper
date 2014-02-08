#ifndef __Q_GL_CANVAS_HPP__
#define __Q_GL_CANVAS_HPP__

#include <memory>
#include <mutex>
#include <QGLWidget>
#include <QPaintEvent>

#include "VideoFrame.hpp"

/// Uses OpenGL to draw an image
class QGLCanvas : public QGLWidget
{
public:
	QGLCanvas(QWidget* parent = NULL);

	/// Sets the image that should be drawn on the canvas
	void setFrame(const std::shared_ptr<VideoFrame>& newFrame);

	void setFrame(QImage* newFrame);

protected:

	/// Paint the canvas
	void paintEvent(QPaintEvent*);

private:

	/// Since img is just a shallow copy, we need to keep the actual frame around
	std::shared_ptr<VideoFrame> frame;

	/// Image used to draw
	std::unique_ptr<QImage> img;

	/// Guarantees nobody is accessing image pixels at the same time
	std::mutex pixelsMutex;
};

#endif
