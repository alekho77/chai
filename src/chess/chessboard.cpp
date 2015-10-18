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

void Chessboard::createChessboard(int size)
{
  const QColor cblight(255, 206, 158);
  const QColor cbdark(209, 139, 71);

  const int cellsize = size / 10;
  const int startcorner = (size - cellsize * 8) / 2;
  const int centersize = (size - cellsize * 8 + 3 * cellsize * 8) / 3;
  const int bandsize = (size - centersize) / 2;

  rectStartCell = QRect(startcorner, startcorner, cellsize, cellsize);

  imgBoard.reset();
  imgBoard.reset(new QImage(size, size, QImage::Format_RGB32));
  imgBoard->fill(cbdark);
  QPainter painter(&*imgBoard);
  painter.fillRect((size - centersize) / 2, (size - centersize) / 2, centersize, centersize, cblight);
  QRect cellrect = rectStartCell;
  painter.setPen(cblight);
  QFont font("Arial Black");
  font.setPixelSize(2 * cellsize / 3);
  painter.setFont(font);
  QFontMetrics fm = painter.fontMetrics();
  for (int i = 0; i < 8; i++)
  {
    QString row = QString::number(8 - i);
    QRect symrect = fm.tightBoundingRect(row);
    painter.drawText((bandsize - symrect.width()) / 2 - symrect.left(), startcorner + (i + 1) * cellsize - (cellsize - symrect.height()) / 2, row);
    for (int j = (i % 2 + 1) % 2; j < 8; j += 2)
    {
      cellrect.moveTo(startcorner + j * cellsize, startcorner + i * cellsize);
      painter.fillRect(cellrect, cbdark);
    }
  }
  for (int i = 0; i < 8; i++)
  {
    QString row = QString('a' + i);
    QRect symrect = fm.tightBoundingRect(row);
    painter.drawText(startcorner + i * cellsize + (cellsize - symrect.width()) / 2 - symrect.left(), size - (bandsize - symrect.height()) / 2, row);
  }
}

void Chessboard::resizeEvent(QResizeEvent * event)
{
  QWidget::resizeEvent(event);
  createChessboard(qMin(event->size().width(), event->size().height()));
}

void Chessboard::paintEvent(QPaintEvent * event)
{
  QPainter painter(this);
  painter.drawImage(0, 0, *imgBoard);
  //QRect r = rect();
  //r.moveTo(0, 0);
  //painter.fillRect(event->rect(), Qt::red);
}
