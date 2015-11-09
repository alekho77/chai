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
  updateChessPieces();
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
  painter.save();
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
  painter.restore();
}

void Chessboard::drawChesspieces(QPainter& painter)
{
  using namespace Chai::Chess;
  painter.save();
  for (auto p : chessPieces) {
    const QImage& img = p.first == hotPos ?
      (p.second.first == Set::white ? *(hotWhiteImages[p.second.second]) : *(hotBlackImages[p.second.second])) :
      (p.second.first == Set::white ? *(whiteImages[p.second.second]) : *(blackImages[p.second.second]));
    const int x = startCell.left() + startCell.width() * (p.first.file - 'a');
    const int y = startCell.top() + startCell.height() * (8 - (p.first.rank - '0'));
    painter.drawImage(x, y, img);
  }
  painter.restore();
}

void Chessboard::drawChessMoves(QPainter& painter)
{
  using namespace Chai::Chess;
  auto hotpiece = chessPieces.find(hotPos);
  if (hotpiece != chessPieces.end())
  {
    painter.save();
    painter.setPen(Qt::black);
    painter.setBrush(QBrush(hotpiece->second.first == Set::white ? Qt::white : Qt::black, Qt::SolidPattern));
    const qreal scale = 0.3;
    const qreal adjx = startCell.width() * (1 - scale) / 2;
    const qreal adjy = startCell.height() * (1 - scale) / 2;
    const Positions pos = arrToVec(chessMachine->CheckMoves(hotPos));
    for (auto p : pos) {
      QRectF rec;
      rec.setX(startCell.left() + startCell.width() * (p.file - 'a'));
      rec.setY(startCell.top() + startCell.height() * (8 - (p.rank - '0')));
      rec.setWidth(startCell.width());
      rec.setHeight(startCell.height());
      rec.adjust(adjx, adjy, -adjx, -adjy);
      painter.drawEllipse(rec);
    }
    painter.restore();
  }
}

void Chessboard::updateChessPieces()
{
  using namespace Chai::Chess;
  chessPieces.clear();
  for (const Piece* p = chessMachine->GetSet(Set::white); p && p->type != Type::bad; ++p) {
    chessPieces[p->position] = { Set::white, p->type };
  }
  for (const Piece* p = chessMachine->GetSet(Set::black); p && p->type != Type::bad; ++p) {
    chessPieces[p->position] = { Set::black, p->type };
  }
}

Positions Chessboard::arrToVec(const Chai::Chess::Position* p) const
{
  using namespace Chai::Chess;
  Positions set;
  if (p) {
    while (*p != BADPOS) {
      set.insert(*(p++));
    }
  }
  return set;
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
  drawChessMoves(painter);
  drawChesspieces(painter);
}

void Chessboard::leaveEvent(QEvent * event)
{
  hotPos = BADPOS;
  //setCursor(Qt::ArrowCursor);
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
//       if (whitePieces.find(hotPos) != whitePieces.end() || blackPieces.find(hotPos) != blackPieces.end()) {
//         //setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor);
//       } else {
//         //setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::ArrowCursor); //Qt::ForbiddenCursor
//       }
    }
    else
    {
      hotPos = BADPOS;
      //setCursor(Qt::ArrowCursor);
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
    //setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor); // Qt::ForbiddenCursor
  }
  QWidget::mousePressEvent(event);
}

void Chessboard::mouseReleaseEvent(QMouseEvent * event)
{
  if (event->x() >= startCell.left() && event->x() < (startCell.left() + 8 * startCell.width())
    && event->y() >= startCell.top() && event->y() < (startCell.top() + 8 * startCell.height()))
  {
    //setCursor(event->buttons().testFlag(Qt::LeftButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor); // Qt::ForbiddenCursor
  }
  QWidget::mouseReleaseEvent(event);
}
