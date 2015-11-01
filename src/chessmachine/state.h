#pragma once
#include "chessmachine.h"

#define PIECE(p,s,t) std::make_pair(p, PieceState({s, t, false, {}}))

#define PAWN(p,s)   PIECE(p, s, Type::pawn)
#define KNIGHT(p,s) PIECE(p, s, Type::knight)
#define BISHOP(p,s) PIECE(p, s, Type::bishop)
#define ROOK(p,s)   PIECE(p, s, Type::rook)
#define QUEEN(p,s)  PIECE(p, s, Type::queen)
#define KING(p,s)   PIECE(p, s, Type::king)

#define WPAWN(p)    PAWN  (p, Set::white)
#define WKNIGHT(p)  KNIGHT(p, Set::white)
#define WBISHOP(p)  BISHOP(p, Set::white)
#define WROOK(p)    ROOK  (p, Set::white)
#define WQUEEN(p)   QUEEN (p, Set::white)
#define WKING(p)    KING  (p, Set::white)

#define BPAWN(p)    PAWN  (p, Set::black)
#define BKNIGHT(p)  KNIGHT(p, Set::black)
#define BBISHOP(p)  BISHOP(p, Set::black)
#define BROOK(p)    ROOK  (p, Set::black)
#define BQUEEN(p)   QUEEN (p, Set::black)
#define BKING(p)    KING  (p, Set::black)

namespace Chai {
  namespace Chess {

    struct PieceState
    {
      Set set;
      Type type;
      bool moved;
      std::set<Postion> moves;
    };

    typedef std::map< Postion, PieceState > Pieces;
    
    struct Move
    {
      Type type;
      Postion from;
      Postion to;
    };

    struct MoveVector
    {
      char df, dr;
    };
    inline Postion operator + (const Postion& pos, const MoveVector& vec) {
      return Postion({ pos.file + vec.df, pos.rank + vec.dr });
    }
    inline Postion& operator += (Postion& pos, const MoveVector& vec) {
      pos.file += vec.df;
      pos.rank += vec.dr;
      return pos;
    }

    class ChessState
    {
    public:
      ChessState();
      ChessState(const ChessState& state, const Move& move);

      Set activeSet;
      boost::optional<Move> lastMove;
      Pieces pieces;

    private:
      void evalMoves();
      void pieceMoves(PieceState& piece, const Postion& pos, const std::set<Postion>& opponent);
      bool addMoveIf(std::set<Postion>& moves, const Postion& pos, Set set = Set::unknown, bool capture = false) const;
    };
  }
}

