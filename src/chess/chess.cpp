#include "chess.h"

chess::chess(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);

  ui.comboBoxGame->addItem("Manual");
  ui.comboBoxGame->addItem("Greedy");
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

void chess::onLogChanged()
{
  ui.textEditLog->ensureCursorVisible();
}

void chess::onMove()
{
  switch (ui.comboBoxGame->currentIndex())
  {
  case 1:
    //ui.lineEditGreedyScore->setText(QString().setNum(ui.widgetChessboard->positionScore()));
    break;
  }
}

void chess::resizeEvent(QResizeEvent * event)
{
  ui.widgetChessboard->setMaximumWidth(ui.centralWidget->height());
  QMainWindow::resizeEvent(event);
}
