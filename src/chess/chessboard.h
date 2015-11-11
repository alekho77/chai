#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QSharedPointer>
#include <QMap>
#include <QVector>
#include <set>
#include <map>
#include "ui_chessboard.h"
#include "../chessmachine/chessmachine.h"

typedef std::map< Chai::Chess::Position, std::pair<Chai::Chess::Set, Chai::Chess::Type> > ChessPieces;
typedef std::set<Chai::Chess::Position> Positions;
typedef QMap< Chai::Chess::Type, QSharedPointer<QImage> > ChessPieceImages;

class Chessboard : public QWidget
{
  Q_OBJECT

public:
  Chessboard(QWidget *parent = 0);
  ~Chessboard();

public slots:
  void newGame();

protected:
  void resizeEvent(QResizeEvent * event) override;
  void paintEvent(QPaintEvent * event) override;
  void leaveEvent(QEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;

private:
  void createChessboard(int size);
  QSharedPointer<QImage> createPieceImage(const QString& filename, qreal scale);
  
  void drawChessboardLabels(QPainter& painter);
  void drawChesspieces(QPainter& painter);
  void drawChessMoves(QPainter& painter);

  void updateChessPieces();
  Positions arrToVec(const Chai::Chess::Position* p) const;
  bool overBoard(int x, int y) const {
    return x >= startCell.left() && x < (startCell.left() + 8 * startCell.width())
      && y >= startCell.top() && y < (startCell.top() + 8 * startCell.height());
  }
  void updateCursor();

  Ui::chessboardClass ui;
  
  const QColor cellLight;
  const QColor cellDark;

  QSharedPointer<QImage> imgBoard;
  QRect startCell;
  Chai::Chess::Position hotPos;
  Chai::Chess::Position dragPos;
  QPoint dragPoint;
  ChessPieceImages whiteImages, hotWhiteImages;
  ChessPieceImages blackImages, hotBlackImages;
  ChessPieces chessPieces;

  QSharedPointer<Chai::Chess::IChessMachine> chessMachine;
};

#endif // CHESSBOARD_H
