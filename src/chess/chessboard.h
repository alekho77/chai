#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QSharedPointer>
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

private:
  void createChessboard(int size);

  Ui::chessboardClass ui;
  QSharedPointer<QImage> imgBoard;
  QRect rectStartCell;
};

#endif // CHESSBOARD_H
