#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QSharedPointer>
#include <QMap>
#include <QVector>
#include "ui_chessboard.h"
#include "../chessmachine/chessmachine.h"

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
  typedef QMap<Chai::Chess::Position, Chai::Chess::Type> ChessPieces;
  ChessPieces arrToVec(const Chai::Chess::Piece* p) const;

  Ui::chessboardClass ui;
  
  const QColor cellLight;
  const QColor cellDark;

  QSharedPointer<QImage> imgBoard;
  QRect startCell;
  Chai::Chess::Position hotPos;
  typedef QMap< Chai::Chess::Type, QSharedPointer<QImage> > ChessPieceImages;
  ChessPieceImages whiteImages, hotWhiteImages;
  ChessPieceImages blackImages, hotBlackImages;
  ChessPieces whitePieces;
  ChessPieces blackPieces;

  QSharedPointer<Chai::Chess::IChessMachine> chessMachine;
};

#endif // CHESSBOARD_H
