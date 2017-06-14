#ifndef CHESS_H
#define CHESS_H

#pragma warning(push,1)
#include <QtWidgets/QMainWindow>
#include "ui_chess.h"
#pragma warning(pop)

class chess : public QMainWindow
{
  Q_OBJECT

public:
  chess(QWidget *parent = 0);
  ~chess();

public slots:
  void newGame();
  void stopGame();
  void onLogChanged();
  void onReadyOk(bool ok);
  void makeMove();

protected:
  void resizeEvent(QResizeEvent * event) override;

private:
  Ui::chessClass ui;
};

#endif // CHESS_H
