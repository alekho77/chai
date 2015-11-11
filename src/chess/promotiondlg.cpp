#include "promotiondlg.h"

PromotionDlg::PromotionDlg(Chai::Chess::Set set, QWidget *parent)
    : QDialog(parent)
{
  ui.setupUi(this);
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
  ui.stackedWidget->setCurrentIndex(set == Chai::Chess::Set::white ? 0 : 1);
}

PromotionDlg::~PromotionDlg()
{

}
