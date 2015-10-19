#include "chessboard.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <QSet>

Chessboard::Chessboard(QWidget *parent)
  : QWidget(parent)
  , cellLight(255, 206, 158)
  , cellDark(209, 139, 71)
  , hotFile(QChar::Null), hotRank(QChar::Null)
{
  ui.setupUi(this);
  
  position.append(QVector<PieceState>({ { true, "pawn",'a','2' },{ true, "pawn",'b','2' },{ true, "pawn",'c','2' },{ true, "pawn",'d','2' },{ true, "pawn",'e','2' },{ true, "pawn",'f','2' },{ true, "pawn",'g','2' },{ true, "pawn",'h','2' } }));
  position.append(QVector<PieceState>({ { true, "rook",'a','1' },{ true, "knight",'b','1' },{ true, "bishop",'c','1' },{ true, "queen",'d','1' },{ true, "king",'e','1' },{ true, "bishop",'f','1' },{ true, "knight",'g','1' },{ true, "rook",'h','1' } }));
  position.append(QVector<PieceState>({ { false, "pawn",'a','7' },{ false, "pawn",'b','7' },{ false, "pawn",'c','7' },{ false, "pawn",'d','7' },{ false, "pawn",'e','7' },{ false, "pawn",'f','7' },{ false, "pawn",'g','7' },{ false, "pawn",'h','7' } }));
  position.append(QVector<PieceState>({ { false, "rook",'a','8' },{ false, "knight",'b','8' },{ false, "bishop",'c','8' },{ false, "queen",'d','8' },{ false, "king",'e','8' },{ false, "bishop",'f','8' },{ false, "knight",'g','8' },{ false, "rook",'h','8' } }));
}

Chessboard::~Chessboard()
{

}

void Chessboard::createChessboard(int size)
{
  const int cellsize = size / 10;
  const int startcorner = (size - cellsize * 8) / 2;
  const int centersize = (size - cellsize * 8 + 3 * cellsize * 8) / 3;

  startCell = QRect(startcorner, startcorner, cellsize, cellsize);

  imgBoard.reset();
  imgBoard.reset(new QImage(size, size, QImage::Format_RGB32));
  imgBoard->fill(cellDark);
  {
    QPainter painter(&*imgBoard);
    painter.fillRect((size - centersize) / 2, (size - centersize) / 2, centersize, centersize, cellLight);
    QRect cellrect = startCell;
    for (int i = 0; i < 8; i++)
      for (int j = (i % 2 + 1) % 2; j < 8; j += 2)
      {
        cellrect.moveTo(startcorner + j * cellsize, startcorner + i * cellsize);
        painter.fillRect(cellrect, cellDark);
      }
  }
  whitePieces.clear();
  blackPieces.clear();
  {
    const QSet<QString> names({"pawn", "knight", "bishop", "rook", "queen", "king"});
    for (auto iter = names.begin(); iter != names.end(); ++iter)
    {
      const qreal scale = 0.6;
      whitePieces[*iter] = createPieceImage(QString(":/chess/Resources/chess-pieces/") + (*iter) + "-w.svg", scale);
      blackPieces[*iter] = createPieceImage(QString(":/chess/Resources/chess-pieces/") + (*iter) + "-b.svg", scale);
    }
  }
}

void Chessboard::drawChessboardLabels(QPainter& painter)
{
  const int bandsize = (imgBoard->width() - 8 * startCell.width()) / 3;
  painter.setPen(cellLight);
  QFont font("Arial Black");
  font.setPixelSize(2 * startCell.width() / 3);
  painter.setFont(font);
  QFontMetrics fm = painter.fontMetrics();
  for (int i = 0; i < 8; i++)
  {
    const QChar r('0' + 8 - i);
    const QChar f('a' + i);
    const QRect rrect = fm.tightBoundingRect(r);
    const QRect frect = fm.tightBoundingRect(f);
    if (hotRank == r)
      painter.setPen(Qt::white);
    else
      painter.setPen(cellLight);
    painter.drawText((bandsize - rrect.width()) / 2 - rrect.left(), startCell.top() + (i + 1) * startCell.height() - (startCell.height() - rrect.height()) / 2, r);
    if (hotFile == f)
      painter.setPen(Qt::white);
    else
      painter.setPen(cellLight);
    painter.drawText(startCell.left() + i * startCell.width() + (startCell.width() - frect.width()) / 2 - frect.left(), imgBoard->height() - (bandsize - frect.height()) / 2, f);
  }
}

QSharedPointer<QImage> Chessboard::createPieceImage(const QString& filename, qreal scale)
{
  QSvgRenderer renderer(filename);
  QSharedPointer<QImage> img(new QImage(startCell.size(), QImage::Format_ARGB32));
  QPainter painter(&*img);
  renderer.render(&painter, QRectF(startCell.width() * (1 - scale) / 2, startCell.height() * (1 - scale) / 2, startCell.width() * scale, startCell.height() * scale));
  return img;
}

void Chessboard::drawChesspieces(QPainter& painter)
{
  for (auto p = position.begin(); p != position.end(); ++p)
  {
    const QImage& img = p->white ? *(whitePieces[p->name]) : *(blackPieces[p->name]);
    int x = startCell.left() + startCell.width() * (p->file.toLatin1() - 'a');
    int y = startCell.top() + startCell.height() * (8 - (p->rank.toLatin1() - '0'));
    painter.drawImage(x, y, img);
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
  drawChessboardLabels(painter);
  drawChesspieces(painter);
}

void Chessboard::leaveEvent(QEvent * event)
{
  hotFile = hotRank = QChar::Null;
  setCursor(Qt::ArrowCursor);
  repaint();
  QWidget::leaveEvent(event);
}

void Chessboard::mouseMoveEvent(QMouseEvent * event)
{
  if (imgBoard)
  {
    if (event->x() >= startCell.left() && event->x() < (startCell.left() + 8 * startCell.width())
      && event->y() >= startCell.top() && event->y() < (startCell.top() + 8 * startCell.height()))
    {
      int x = (event->x() - startCell.left()) / startCell.width();
      Q_ASSERT(x >= 0 && x < 8);
      hotFile = QChar('a' + x);
      int y = (event->y() - startCell.top()) / startCell.height();
      Q_ASSERT(y >= 0 && y < 8);
      hotRank = QChar('0' + 8 - y);
      setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor); // Qt::ForbiddenCursor
    }
    else
    {
      hotFile = hotRank = QChar::Null;
      setCursor(Qt::ArrowCursor);
    }
    repaint();
  }
  QWidget::mouseMoveEvent(event);
}

void Chessboard::mousePressEvent(QMouseEvent * event)
{
  if (event->x() >= startCell.left() && event->x() < (startCell.left() + 8 * startCell.width())
    && event->y() >= startCell.top() && event->y() < (startCell.top() + 8 * startCell.height()))
  {
    setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor); // Qt::ForbiddenCursor
  }
  QWidget::mousePressEvent(event);
}

void Chessboard::mouseReleaseEvent(QMouseEvent * event)
{
  if (event->x() >= startCell.left() && event->x() < (startCell.left() + 8 * startCell.width())
    && event->y() >= startCell.top() && event->y() < (startCell.top() + 8 * startCell.height()))
  {
    setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor); // Qt::ForbiddenCursor
  }
  QWidget::mouseReleaseEvent(event);
}
