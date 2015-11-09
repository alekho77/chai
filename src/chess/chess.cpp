#include "chess.h"

chess::chess(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);

  ui.comboBoxGame->addItem("Manual");
}

chess::~chess()
{

}

void chess::newGame()
{
  ui.groupBoxOptions->setEnabled(false);
  ui.pushButtonStopGame->setEnabled(true);
  ui.widgetChessboard->setEnabled(true);
}

void chess::stopGame()
{
  ui.groupBoxOptions->setEnabled(true);
  ui.pushButtonStopGame->setEnabled(false);
  ui.widgetChessboard->setEnabled(false);
}

void chess::resizeEvent(QResizeEvent * event)
{
  //ui.plainTextEditLog->appendPlainText(QString::number(ui.centralWidget->height()));
  ui.widgetChessboard->setMaximumWidth(ui.centralWidget->height());
  QMainWindow::resizeEvent(event);
}
