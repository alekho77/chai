#include "chessboard.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

Chessboard::Chessboard(QWidget *parent)
  : QWidget(parent)
{
  ui.setupUi(this);
}

Chessboard::~Chessboard()
{

}

void Chessboard::resizeEvent(QResizeEvent * event)
{
  int size = qMin(event->size().width(), event->size().height());
  QWidget::resizeEvent(event);
  imgBoard = QImage(size, size, QImage::Format_RGB32);
  imgBoard.fill(Qt::red);
}

void Chessboard::paintEvent(QPaintEvent * event)
{
  QPainter painter(this);
  painter.drawImage(0, 0, imgBoard);
  //QRect r = rect();
  //r.moveTo(0, 0);
  //painter.fillRect(event->rect(), Qt::red);
}
