#include "chess.h"
#include <QResizeEvent>

chess::chess(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);
}

chess::~chess()
{

}

void chess::resizeEvent(QResizeEvent * event)
{
  QMainWindow::resizeEvent(event);
  int size = qMin(event->size().width(), event->size().height());
  //ui.widgetChessboard->resize(size, size);
}
