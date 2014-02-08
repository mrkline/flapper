#-------------------------------------------------
#
# Project created by QtCreator 2013-07-01T11:50:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

TARGET = flapper
TEMPLATE = app

CONFIG += c++11 debug_and_release

LIBS += -lX11

QMAKE_CXXFLAGS += -Wall -Wextra

SOURCES += main.cpp \
DisplayWindow.cpp \
QGLCanvas.cpp \
X11ScreenIO.cpp

HEADERS  += DisplayWindow.hpp \
QGLCanvas.hpp \
VideoFrame.hpp \
ScreenIO.hpp \
X11ScreenIO.hpp \
Exceptions.hpp \
MKMath.hpp \

FORMS    += DisplayWindow.ui