#include "chessboard.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <QSet>
#include <QPair>

Chessboard::Chessboard(QWidget *parent)
  : QWidget(parent)
  , cellLight(255, 206, 158)
  , cellDark(209, 139, 71)
  , hotPos(BADPOS)
{
  ui.setupUi(this);
  
  using namespace Chai::Chess;
  chessMachine.reset(CreateChessMachine(), DeleteChessMachine);
}

Chessboard::~Chessboard()
{
}

void Chessboard::newGame()
{
  chessMachine->Start();
  whitePieces = arrToVec(chessMachine->GetSet(Chai::Chess::Set::white));
  blackPieces = arrToVec(chessMachine->GetSet(Chai::Chess::Set::black));
  repaint();
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
  whiteImages.clear();
  hotWhiteImages.clear();
  blackImages.clear();
  hotBlackImages.clear();
  {
    using namespace Chai::Chess;
    const QMap<Type,QString> names({{Type::pawn,"pawn"}, {Type::knight,"knight"}, {Type::bishop,"bishop"}, {Type::rook,"rook"}, {Type::queen,"queen"}, {Type::king,"king"}});
    for (auto iter = names.begin(); iter != names.end(); ++iter)
    {
      whiteImages[iter.key()] = createPieceImage(QString(":/chess/Resources/chess-pieces/") + (*iter) + "-w.svg", 0.8);
      hotWhiteImages[iter.key()] = createPieceImage(QString(":/chess/Resources/chess-pieces/") + (*iter) + "-w.svg", 0.95);
      blackImages[iter.key()] = createPieceImage(QString(":/chess/Resources/chess-pieces/") + (*iter) + "-b.svg", 0.8);
      hotBlackImages[iter.key()] = createPieceImage(QString(":/chess/Resources/chess-pieces/") + (*iter) + "-b.svg", 0.95);
    }
  }
}

QSharedPointer<QImage> Chessboard::createPieceImage(const QString& filename, qreal scale)
{
  QSvgRenderer renderer(filename);
  QSharedPointer<QImage> img(new QImage(startCell.size(), QImage::Format_ARGB32));
  img->fill(Qt::transparent);
  QPainter painter(&*img);
  renderer.render(&painter, QRectF(startCell.width() * (1 - scale) / 2, startCell.height() * (1 - scale) / 2, startCell.width() * scale, startCell.height() * scale));
  return img;
}

void Chessboard::drawChessboardLabels(QPainter& painter)
{
  const int bandsize = (imgBoard->width() - 8 * startCell.width()) / 3;
  painter.setPen(cellLight);
  QFont font("Arial Black");
  font.setPixelSize(2 * startCell.width() / 3);
  painter.setFont(font);
  QFontMetrics fm = painter.fontMetrics();
  for (char i = 0; i < 8; i++)
  {
    const Chai::Chess::Position pos = {'a' + i, '0' + 8 - i};
    const QRect rrect = fm.tightBoundingRect(QString(pos.rank));
    const QRect frect = fm.tightBoundingRect(QString(pos.file));
    painter.setPen(hotPos.rank == pos.rank ? Qt::white : cellLight);
    painter.drawText((bandsize - rrect.width()) / 2 - rrect.left(), startCell.top() + (i + 1) * startCell.height() - (startCell.height() - rrect.height()) / 2, QString(pos.rank));
    painter.setPen(hotPos.file == pos.file ? Qt::white : cellLight);
    painter.drawText(startCell.left() + i * startCell.width() + (startCell.width() - frect.width()) / 2 - frect.left(), imgBoard->height() - (bandsize - frect.height()) / 2, QString(pos.file));
  }
}

void Chessboard::drawChesspieces(QPainter& painter)
{
  for (auto p = whitePieces.begin(); p != whitePieces.end(); ++p) {
    const QImage& img = p.key() == hotPos ? *(hotWhiteImages[p.value()]) : *(whiteImages[p.value()]);
    int x = startCell.left() + startCell.width() * (p.key().file - 'a');
    int y = startCell.top() + startCell.height() * (8 - (p.key().rank - '0'));
    painter.drawImage(x, y, img);
  }
  for (auto p = blackPieces.begin(); p != blackPieces.end(); ++p) {
    const QImage& img = p.key() == hotPos ? *(hotBlackImages[p.value()]) : *(blackImages[p.value()]);
    int x = startCell.left() + startCell.width() * (p.key().file - 'a');
    int y = startCell.top() + startCell.height() * (8 - (p.key().rank - '0'));
    painter.drawImage(x, y, img);
  }
}

Chessboard::ChessPieces Chessboard::arrToVec(const Chai::Chess::Piece* p) const
{
  using namespace Chai::Chess;
  ChessPieces map;
  if (p) {
    for (; p->type != Type::bad; ++p) {
      map[p->position] = p->type;
    }
  }
  return map;
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
  hotPos = BADPOS;
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
      char x = (event->x() - startCell.left()) / startCell.width();
      char y = (event->y() - startCell.top()) / startCell.height();
      Q_ASSERT(x >= 0 && x < 8 && y >= 0 && y < 8);
      hotPos = {'a' + x, '0' + 8 - y};
      setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor); // Qt::ForbiddenCursor
    }
    else
    {
      hotPos = BADPOS;
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
