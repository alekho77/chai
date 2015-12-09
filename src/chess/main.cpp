#include "chess.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
  QCoreApplication::addLibraryPath("platforms");

  QApplication a(argc, argv);
  chess w;
  w.show();
  return a.exec();
}
