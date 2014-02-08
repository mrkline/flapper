#include <QApplication>

#include "DisplayWindow.hpp"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DisplayWindow w;
	w.show();

	return a.exec();
}
