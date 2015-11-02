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
      std::set<Position> moves;
      bool operator == (const PieceState& that) const {
        return set == that.set && type == that.type && moved == that.moved;
      }
    };

    typedef std::map< Position, PieceState > Pieces;
    
    struct Move
    {
      Type type;
      Position from;
      Position to;
    };

    struct MoveVector
    {
      char df, dr;
    };
    inline Position operator + (const Position& pos, const MoveVector& vec) {
      return Position({ pos.file + vec.df, pos.rank + vec.dr });
    }
    inline Position& operator += (Position& pos, const MoveVector& vec) {
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
      static std::set<Position> pieceMoves(const Pieces& pieces, const Position& pos, const std::set<Position>& opponent = {});
      static bool addMoveIf(const Pieces& pieces, std::set<Position>& moves, const Position& pos, Set set = Set::unknown, bool capture = false);
      static bool testPath(const Pieces& pieces, const std::set<Position>& attack, const std::vector<Position>& path);
      static bool testPiece(const Pieces& pieces, const std::pair<Position, PieceState>& piece);
    };
  }
}

