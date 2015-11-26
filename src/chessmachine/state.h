#pragma once

#define PIECE(p,s,t) std::make_pair(p, PieceState({ s, t, false, {} }))

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

    typedef boost::container::flat_set<Position> SetMoves;
    typedef boost::container::small_vector<Position, 3> PiecePath;

    struct PieceState
    {
      Set set;
      Type type;
      bool moved;
      PieceMoves moves;

      bool operator == (const PieceState& that) const {
        return set == that.set && type == that.type && moved == that.moved;
      }
      bool isMove(const Position& to) const {
        return std::binary_search(moves.begin(), moves.end(), to);
      }
    };

    struct PieceStates
    {
      typedef boost::container::flat_map< Position, PieceState > pieces_map;
      
      pieces_map pieces;

      const PieceState& operator[] (const Position& pos) const {
        return pieces.at(pos);
      }
      void erase(const Position& pos) {
        pieces.erase(pos);
      }
      void set(const Position& pos, const PieceState& state) {
        pieces[pos] = state;
      }
      const boost::optional<const PieceState> get(const Position& pos) const {
        auto p = pieces.find(pos);
        return p != pieces.end() ? p->second : boost::optional<const PieceState>();
      }
      bool test(const Position& pos) const {
        return pieces.find(pos) != pieces.end();
      }
    };
    
    struct Move
    {
      Type type;
      Position from;
      Position to;
      Type promotion;
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
      
      ChessState MakeMove(const Move& move) const;

      Set activeSet;
      boost::optional<Move> lastMove;
      PieceStates pieces;

    private:
      ChessState(Set set, const Move& move, const PieceStates& pieces);

      void evalMoves(boost::optional<Move> xmove);
      static PieceMoves pieceMoves(const PieceStates& pieces, const Position& pos, boost::optional<Move> xmove, const SetMoves& xmoves = SetMoves());
      static bool addMoveIf(const PieceStates& pieces, PieceMoves& moves, const Position& pos, Set set = Set::unknown, bool capture = false);
      static bool testPath(const PieceStates& pieces, const SetMoves& xmoves, const PiecePath& path);
      static bool testPiece(const PieceStates& pieces, const std::pair<Position, PieceState>& piece);
    };
  }
}

