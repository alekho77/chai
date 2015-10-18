#include "chess.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	chess w;
	w.show();
	return a.exec();
}
