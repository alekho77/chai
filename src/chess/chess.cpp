#include "chess.h"

chess::chess(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);
}

chess::~chess()
{

}

void chess::newGame()
{
  ui.comboBoxGame->setEnabled(false);
  ui.pushButtonStopGame->setEnabled(true);
  ui.frameEngine->setEnabled(ui.comboBoxGame->currentIndex() > 0);
  ui.widgetChessboard->setEnabled(true);
  ui.widgetChessboard->newGame(ui.comboBoxGame->currentText());
}

void chess::stopGame()
{
  ui.comboBoxGame->setEnabled(true);
  ui.pushButtonStopGame->setEnabled(false);
  ui.frameEngine->setEnabled(false);
  ui.widgetChessboard->setEnabled(false);
}

void chess::onLogChanged()
{
  ui.textEditLog->ensureCursorVisible();
}

void chess::setCurrentScore(float score)
{
  ui.lineEditCurScore->setText(QString().setNum(score, 'f', 3));
}

void chess::resizeEvent(QResizeEvent * event)
{
  ui.widgetChessboard->setMaximumWidth(ui.centralWidget->height());
  QMainWindow::resizeEvent(event);
}
