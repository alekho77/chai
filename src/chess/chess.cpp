#include "chess.h"

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
  //ui.plainTextEditLog->appendPlainText(QString::number(ui.centralWidget->height()));
  ui.widgetChessboard->setMaximumWidth(ui.centralWidget->height());
  QMainWindow::resizeEvent(event);
}
