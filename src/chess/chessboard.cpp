#include "chessboard.h"
#include "promotiondlg.h"

#include <chessmachine/machine.h>
#include <ChessEngineGreedy/engine.h>

#pragma warning(push,1)
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <QSet>
#include <QPair>
#pragma warning(pop)

#include <boost/make_shared.hpp>

Chessboard::Chessboard(QWidget *parent)
  : QWidget(parent)
  , cellLight(255, 206, 158)
  , cellDark(209, 139, 71)
  , hotPos(BADPOS)
  , dragPos(BADPOS)
  , moveCount(0)
  , engineTimer(0)
  , maxDepth(0)
  , boardLayout(BlackTop)
  , autoRotate(false)
{
  ui.setupUi(this);
  
  using namespace Chai::Chess;
  chessMachine = boost::make_shared<Chai::Chess::ChessMachine>();
}

Chessboard::~Chessboard()
{
  stopGame();
}

void Chessboard::newGame(QString engine)
{
  using namespace Chai::Chess;
  chessMachine->Start();
  updateChessPieces();
  repaint();
  moveCount = 1;
  if (engine == "Greedy") {
    chessEngine = boost::make_shared<Chai::Chess::GreedyEngine>();
  }
  afterMove(false);
}

void Chessboard::stopGame()
{
  if (engineTimer) {
    killTimer(engineTimer);
    engineTimer = 0;
  }
  chessEngine.reset();
}

void Chessboard::setDepth(int depth)
{
  maxDepth = depth;
}

void Chessboard::abortEval()
{
  if (engineTimer) {
    if (chessEngine) {
      chessEngine->Stop();
    }
  } else {
    afterMove(false);
  }
}

void Chessboard::makeMove(QString move)
{
  if (chessMachine->Move(move.toStdString())) {
    afterMove(true);
    dragPos = BADPOS;
    repaint();
    updateCursor();
  }
}

void Chessboard::rotateBoard()
{
  boardLayout = boardLayout == BlackTop ? WhiteTop : BlackTop;
  dragPos = BADPOS;
  repaint();
  updateCursor();
}

void Chessboard::setAutoRotate(bool rot)
{
  autoRotate = rot;
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
    const QRect rrect = fm.tightBoundingRect(QString(pos.rank()));
    const QRect frect = fm.tightBoundingRect(QString(pos.file()));
    painter.setPen(hotPos.rank() == pos.rank() ? Qt::white : cellLight);
    if (boardLayout == BlackTop) {
      painter.drawText((bandsize - rrect.width()) / 2 - rrect.left(), startCell.top() + (i + 1) * startCell.height() - (startCell.height() - rrect.height()) / 2, QString(pos.rank()));
    } else {
      painter.drawText((bandsize - rrect.width()) / 2 - rrect.left(), startCell.top() + (8 - i) * startCell.height() - (startCell.height() - rrect.height()) / 2, QString(pos.rank()));
    }
    painter.setPen(hotPos.file() == pos.file() ? Qt::white : cellLight);
    if (boardLayout == BlackTop) {
      painter.drawText(startCell.left() + i * startCell.width() + (startCell.width() - frect.width()) / 2 - frect.left(), imgBoard->height() - (bandsize - frect.height()) / 2, QString(pos.file()));
    } else {
      painter.drawText(startCell.left() + (7 - i) * startCell.width() + (startCell.width() - frect.width()) / 2 - frect.left(), imgBoard->height() - (bandsize - frect.height()) / 2, QString(pos.file()));
    }
  }
  painter.restore();
}

void Chessboard::drawChesspieces(QPainter& painter)
{
  using namespace Chai::Chess;
  painter.save();
  for (auto p : chessPieces) {
    if (p.first != dragPos)
    {
      const QImage& img = p.first == hotPos ?
        (p.second.first == Set::white ? *(hotWhiteImages[p.second.second]) : *(hotBlackImages[p.second.second])) :
        (p.second.first == Set::white ? *(whiteImages[p.second.second]) : *(blackImages[p.second.second]));
      int x, y;
      if (boardLayout == BlackTop) {
        x = startCell.left() + startCell.width() * p.first.x();
        y = startCell.top() + startCell.height() * (7 - p.first.y());
      } else {
        x = startCell.left() + startCell.width() * (7 - p.first.x());
        y = startCell.top() + startCell.height() * p.first.y();
      }
      painter.drawImage(x, y, img);
    }
  }
  if (dragPos != BADPOS) {
    auto p = chessPieces.at(dragPos);
    const QImage& img = p.first == Set::white ? *(hotWhiteImages[p.second]) : *(hotBlackImages[p.second]);
    painter.drawImage(dragPoint, img);
  }
  painter.restore();
}

void Chessboard::drawChessMoves(QPainter& painter)
{
  using namespace Chai::Chess;
  auto piece = chessPieces.find(dragPos != BADPOS ? dragPos : hotPos);
  if (piece != chessPieces.end())
  {
    painter.save();
    painter.setPen(Qt::black);
    painter.setBrush(QBrush(piece->second.first == Set::white ? Qt::white : Qt::black, Qt::SolidPattern));
    const qreal scalec = 0.3;
    const qreal adjxc = startCell.width() * (1 - scalec) / 2;
    const qreal adjyc = startCell.height() * (1 - scalec) / 2;
    const qreal scalel = 0.5;
    const qreal adjxl = startCell.width() * (1 - scalel) / 2;
    const qreal adjyl = startCell.height() * (1 - scalel) / 2;
    for (auto p : chessMachine->EnumMoves(dragPos != BADPOS ? dragPos : hotPos)) {
      QRectF rec;
      if (boardLayout == BlackTop) {
        rec.setX(startCell.left() + startCell.width() * p.x());
        rec.setY(startCell.top() + startCell.height() * (7 - p.y()));
      } else {
        rec.setX(startCell.left() + startCell.width() * (7 - p.x()));
        rec.setY(startCell.top() + startCell.height() * p.y());
      }
      rec.setWidth(startCell.width());
      rec.setHeight(startCell.height());
      if (chessPieces.find(p) != chessPieces.end() || (piece->second.second == Type::pawn && p.file() != piece->first.file()))
      {
        rec.adjust(adjxl, adjyl, -adjxl, -adjyl);
        painter.save();
        QPen pen(Qt::red);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidthF(startCell.width() * 0.2);
        painter.setPen(pen);
        painter.drawLine(rec.topLeft(), rec.bottomRight());
        painter.drawLine(rec.bottomLeft(), rec.topRight());
        painter.restore();
      }
      else
      {
        rec.adjust(adjxc, adjyc, -adjxc, -adjyc);
        painter.drawEllipse(rec);
      }
    }
    painter.restore();
  }
}

void Chessboard::updateChessPieces()
{
  using namespace Chai::Chess;
  chessPieces.clear();
  for (const auto& p : chessMachine->GetSet(Set::white)) {
    chessPieces[p.position] = { Set::white, p.type };
  }
  for (const auto& p : chessMachine->GetSet(Set::black)) {
    chessPieces[p.position] = { Set::black, p.type };
  }
}

void Chessboard::updateCursor()
{
  using namespace Chai::Chess;
  if (dragPos != BADPOS) {
    const PieceMoves pos = chessMachine->EnumMoves(dragPos);
    setCursor(std::binary_search(pos.begin(), pos.end(), hotPos) ? Qt::ClosedHandCursor : Qt::ForbiddenCursor);
  }
  else
  {
    auto piece = chessPieces.find(hotPos);
    if (piece != chessPieces.end()) {
      if (piece->second.first == chessMachine->CurrentPlayer()) {
        setCursor(Qt::OpenHandCursor);
      }
      else {
        setCursor(Qt::PointingHandCursor);
      }
    }
    else {
      setCursor(Qt::ArrowCursor);
    }
  }
}

void Chessboard::afterMove(bool shownot)
{
  using namespace Chai::Chess;

  if (engineTimer) {
    if (chessEngine) {
      chessEngine->Stop();
      chessEngine->ProcessInfo(this);
    }
  }

  if (shownot) {
    QString notation = QString::fromStdString(chessMachine->LastMoveNotation());
    switch (chessMachine->CheckStatus())
    {
    case Status::check: notation += "+"; break;
    case Status::checkmate: notation += "#"; break;
    case Status::stalemate: notation += "="; break;
    }
    if (chessMachine->CurrentPlayer() == Set::black) {
      const int c = (notation.length() < 8 ? 8 - notation.length() : 0) + 1;
      notation = (moveCount < 10 ? "&nbsp;" : "") + QString::number(moveCount) + "." + notation;
      for (int i = 0; i < c; i++) {
        notation += "&nbsp;";
      }
    }
    notation = "<code>" + notation + "</code>";
    if (chessMachine->CurrentPlayer() == Set::white) {
      notation += QString("<br />");
      moveCount++;
    }
    emit updateLog(notation);
    updateChessPieces();
  }

  emit currentPlayer(chessMachine->CurrentPlayer() == Set::white ? "White" : "Black");
  if (chessEngine) {
    emit currentScore(QString().setNum(chessEngine->EvalPosition(*chessMachine), 'f', 3));
    emit nodesSearched("...");
    emit bestScore("...");
    emit bestMove("...");
    emit readyOk(false);
    if (chessEngine->Start(*chessMachine, maxDepth)) {
      engineTimer = startTimer(300);
    }
  } else {
    emit currentScore("n/a");
    emit nodesSearched("n/a");
    emit bestScore("n/a");
    emit bestMove("n/a");
  }

  if (autoRotate) {
    boardLayout = chessMachine->CurrentPlayer() == Set::white ? BlackTop : WhiteTop;
  }
}

void Chessboard::resizeEvent(QResizeEvent * event)
{
  QWidget::resizeEvent(event);
  createChessboard(qMin(event->size().width(), event->size().height()));
}

void Chessboard::paintEvent(QPaintEvent * /*event*/)
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
  dragPos = BADPOS;
  updateCursor();
  repaint();
  QWidget::leaveEvent(event);
}

void Chessboard::mouseMoveEvent(QMouseEvent * event)
{
  if (imgBoard)
  {
    if (overBoard(event->x(), event->y()))
    {
      char x = static_cast<char>( (event->x() - startCell.left()) / startCell.width() );
      char y = static_cast<char>( (event->y() - startCell.top()) / startCell.height() );
      assert(x >= 0 && x < 8 && y >= 0 && y < 8);
      if (boardLayout == BlackTop) {
        hotPos = { 'a' + x, '1' + (7 - y) };
      } else {
        hotPos = { 'a' + (7 - x), '1' + y };
      }
    }
    else
    {
      hotPos = BADPOS;
    }
    dragPoint = event->pos() - QPoint(startCell.width() / 2, startCell.height() / 2);
    updateCursor();
    repaint();
  }
  QWidget::mouseMoveEvent(event);
}

void Chessboard::mousePressEvent(QMouseEvent * event)
{
  if (dragPos == BADPOS && event->buttons().testFlag(Qt::LeftButton))
  {
    auto piece = chessPieces.find(hotPos);
    if (piece != chessPieces.end() && piece->second.first == chessMachine->CurrentPlayer())
    {
      dragPos = hotPos;
    }
    repaint();
    updateCursor();
  }
  QWidget::mousePressEvent(event);
}

void Chessboard::mouseReleaseEvent(QMouseEvent * event)
{
  using namespace Chai::Chess;
  if (dragPos != BADPOS && !event->buttons().testFlag(Qt::LeftButton))
  {
    const PieceMoves pos = chessMachine->EnumMoves(dragPos);
    if (std::binary_search(pos.begin(), pos.end(), hotPos)) {
      using namespace Chai::Chess;
      auto piece = chessPieces.at(dragPos);
      Type promotion = Type::bad;
      Position from = dragPos;
      Position to = hotPos;
      if (piece.second == Type::pawn && ((piece.first == Set::white && to.rank() == '8') || (piece.first == Set::black && to.rank() == '1'))) {
        PromotionDlg dlg(chessMachine->CurrentPlayer(), this);
        int result = dlg.exec();
        if (result != QDialog::Rejected) {
          promotion = static_cast<Type>(result);
        }
      }
      if (chessMachine->Move(piece.second, from, to, promotion)) {
        afterMove(true);
      }
    }
    dragPos = BADPOS;
    repaint();
    updateCursor();
  }
  QWidget::mouseReleaseEvent(event);
}

void Chessboard::timerEvent(QTimerEvent * event)
{
  if (event->timerId() == engineTimer && chessEngine) {
    chessEngine->ProcessInfo(this);
    //QApplication::beep();
  }
}

void Chessboard::NodesSearched(size_t nodes)
{
  emit nodesSearched(QString().setNum(nodes));
}

void Chessboard::ReadyOk()
{
  if (engineTimer) {
    killTimer(engineTimer);
    engineTimer = 0;
  }
  emit readyOk(true);
}

void Chessboard::BestMove(std::string notation)
{
  emit bestMove(QString::fromStdString(notation));
}

void Chessboard::BestScore(float score)
{
  emit bestScore(QString().setNum(score, 'f', 3));
}
