#ifndef CHESS_H
#define CHESS_H

#include <QtWidgets/QMainWindow>
#include "ui_chess.h"

class chess : public QMainWindow
{
  Q_OBJECT

public:
  chess(QWidget *parent = 0);
  ~chess();

protected:

private:
  Ui::chessClass ui;
};

#endif // CHESS_H
