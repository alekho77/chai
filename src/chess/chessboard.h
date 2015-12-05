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

class Chessboard : public QWidget, protected Chai::Chess::IInfoCall
{
  Q_OBJECT

  enum BoardLayout { BlackTop, WhiteTop };

public:
  Chessboard(QWidget *parent = 0);
  ~Chessboard();

public slots:
  void newGame(QString engine);
  void stopGame();
  void setDepth(int depth);
  void abortEval();
  void makeMove(QString move);
  void rotateBoard();

protected:
  void resizeEvent(QResizeEvent * event) override;
  void paintEvent(QPaintEvent * event) override;
  void leaveEvent(QEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void timerEvent(QTimerEvent * event) override;

  // Messages sent during the search
  void NodesSearched(size_t nodes) override;
  void NodesPerSecond(int nps) override {}
  // Messages sent after the search
  void ReadyOk() override;
  void BestMove(std::string notation) override;
  void BestScore(float score) override; // in pawns

signals:
  void updateLog(QString str);
  void currentPlayer(QString player);
  void currentScore(QString score);
  void bestMove(QString move);
  void bestScore(QString score);
  void nodesSearched(QString nodes);
  void readyOk(bool);

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
  void afterMove(bool shownot);

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

  int engineTimer;
  int maxDepth;
  BoardLayout boardLayout;
};

#endif // CHESSBOARD_H
