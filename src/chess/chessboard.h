#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QSharedPointer>
#include <QMap>
#include "ui_chessboard.h"

class Chessboard : public QWidget
{
  Q_OBJECT

public:
  Chessboard(QWidget *parent = 0);
  ~Chessboard();

protected:
  void resizeEvent(QResizeEvent * event) override;
  void paintEvent(QPaintEvent * event) override;
  void leaveEvent(QEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;

private:
  void createChessboard(int size);
  void drawChessboardLabels(QPainter& painter);
  QSharedPointer<QImage> createPieceImage(const QString& filename, qreal scale);

  Ui::chessboardClass ui;
  
  const QColor cellLight;
  const QColor cellDark;

  QSharedPointer<QImage> imgBoard;
  QRect startCell;
  QChar hotRank; // Rank - A row of the chessboard.In algebraic notation, ranks are numbered 1–8 starting from White's side of the board
  QChar hotFile; // File - a column of the chessboard. A specific file are named using its position in a–h.
  typedef QMap< QString, QSharedPointer<QImage> > ChessPiecesMap;
  ChessPiecesMap whitePieces;
  ChessPiecesMap blackPieces;
};

#endif // CHESSBOARD_H
