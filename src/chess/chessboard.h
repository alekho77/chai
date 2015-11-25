#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QSharedPointer>
#include <QMap>
#include <QVector>
#include <map>
#include "ui_chessboard.h"
#include "chessmachine.h"

typedef std::map< Chai::Chess::Position, std::pair<Chai::Chess::Set, Chai::Chess::Type> > ChessPieces;
typedef QMap< Chai::Chess::Type, QSharedPointer<QImage> > ChessPieceImages;

class Chessboard : public QWidget
{
  Q_OBJECT

public:
  Chessboard(QWidget *parent = 0);
  ~Chessboard();

public slots:
  void newGame(QString engine);
  void stopGame();

protected:
  void resizeEvent(QResizeEvent * event) override;
  void paintEvent(QPaintEvent * event) override;
  void leaveEvent(QEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;

signals:
  void updateLog(QString str);
  void currentScore(float score);

private:
  void createChessboard(int size);
  QSharedPointer<QImage> createPieceImage(const QString& filename, qreal scale);
  
  void drawChessboardLabels(QPainter& painter);
  void drawChesspieces(QPainter& painter);
  void drawChessMoves(QPainter& painter);

  void updateChessPieces();
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
  int moveCount;

  boost::shared_ptr<Chai::Chess::IMachine> chessMachine;
  boost::shared_ptr<Chai::Chess::IEngine>  chessEngine;
};

#endif // CHESSBOARD_H
