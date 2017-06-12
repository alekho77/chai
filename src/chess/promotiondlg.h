#ifndef PROMOTIONDLG_H
#define PROMOTIONDLG_H

#include <Interfaces/chessmachine.h>

#include <QDialog>
#include "ui_promotiondlg.h"

class PromotionDlg : public QDialog
{
  Q_OBJECT

public:
  PromotionDlg(Chai::Chess::Set set, QWidget *parent = 0);
  ~PromotionDlg();

private slots:
  void selectKnight() { done(static_cast<int>(Chai::Chess::Type::knight)); }
  void selectBishop() { done(static_cast<int>(Chai::Chess::Type::bishop)); }
  void selectRook() { done(static_cast<int>(Chai::Chess::Type::rook)); }
  void selectQueen() { done(static_cast<int>(Chai::Chess::Type::queen)); }

private:
  Ui::promotionClass ui;
};

#endif // PROMOTIONDLG_H
