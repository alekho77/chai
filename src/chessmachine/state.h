#pragma once
#include "chessmachine.h"

#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/flat_set.hpp>

#include <array>

#define PIECE(p,s,t) std::make_pair(p, PieceState(s, t))

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

    typedef boost::container::small_vector<Position, 4> PiecePath;

    struct StateMove
    {
      Type type;
      Position from;
      Position to;
      Type promotion;
    };

    struct PieceState
    {
      PieceState() : set(Set::unknown), type(Type::bad), moved(false) {}
      PieceState(Set s, Type t) : set(s), type(t), moved(false) {}
      PieceState(Set s, Type t, bool m) : set(s), type(t), moved(m) {}
      Set set;
      Type type;
      bool moved;
      PieceMoves moves;

      bool operator == (const PieceState& that) const { return set == that.set && type == that.type && moved == that.moved; }
      bool isMove(const Position& to) const { return std::binary_search(moves.begin(), moves.end(), to); }
      bool valid() const { return type != Type::bad; }
    };

    class Board
    {
      typedef std::array< std::pair<Position, PieceState>, 64> board_type;
    public:
      Board(std::initializer_list< std::pair<Position, PieceState> > il);

      class iterator : public std::iterator_traits<board_type::iterator> {
      public:
        iterator(const board_type& b, const board_type::iterator& i) : board(b), iter(i) {}
        iterator& operator ++ ();
        auto& operator * () const { return *iter; }
        bool operator == (const iterator& other) const { return iter == other.iter; }
        bool operator != (const iterator& other) const { return iter != other.iter; }
        bool operator < (const iterator& other) const { return iter < other.iter; }
      private:
        const board_type& board;
        board_type::iterator iter;
      };
      class const_iterator : public std::iterator_traits<board_type::const_iterator> {
      public:
        const_iterator(const board_type& b, const board_type::const_iterator& i) : board(b), iter(i) {}
        const_iterator& operator ++ ();
        const auto& operator * () const { return *iter; }
        bool operator == (const const_iterator& other) const { return iter == other.iter; }
        bool operator != (const const_iterator& other) const { return iter != other.iter; }
        bool operator < (const const_iterator& other) const { return iter < other.iter; }
      private:
        const board_type& board;
        board_type::const_iterator iter;
      };

      iterator begin() noexcept;
      iterator end() noexcept { return{ pieces, pieces.end() }; }
      const_iterator begin() const noexcept;
      const_iterator end() const noexcept { return{ pieces, pieces.cend() }; }

      const PieceState& operator[] (const Position& pos) const { return get(pos); }
      bool test(const Position& pos) const { return get(pos).valid(); }
      Position king(Set set) const { return set == Set::white ? whiteKing : blackKing; }
      void move(Position from, Position to, Type promotion = Type::bad);
      void erase(const Position& pos);

    private:
      const PieceState& get(const Position& pos) const { return pieces[pos.pos()].second; }
      void set(const Position& pos, const PieceState& state);

      Position whiteKing;
      Position blackKing;
      board_type pieces;
    };

    struct MoveVector
    {
      char df, dr;
    };
    inline Position operator + (const Position& pos, const MoveVector& vec) {
      return Position({ pos.file() + vec.df, pos.rank() + vec.dr });
    }
    inline void operator += (Position& pos, const MoveVector& vec) {
      pos = { pos.file() + vec.df, pos.rank() + vec.dr };
    }

    class ChessState
    {
      typedef boost::container::flat_set<Position> SetMoves;
    public:
      ChessState();
      
      ChessState MakeMove(const StateMove& move) const;

      Set activeSet;
      boost::optional<StateMove> lastMove;
      Board pieces;

    private:
      ChessState(Set set, const StateMove& move, const Board& pieces);

      void evalMoves(boost::optional<StateMove> xmove);
      static PieceMoves pieceMoves(const Board& pieces, const Position& pos, boost::optional<StateMove> xmove, const SetMoves& xmoves = SetMoves());
      static bool addMoveIf(const Board& pieces, PieceMoves& moves, const Position& pos, Set set = Set::unknown, bool capture = false);
      static bool testPath(const Board& pieces, const SetMoves& xmoves, const PiecePath& path);
      static bool testPiece(const Board& pieces, const std::pair<Position, PieceState>& piece);
    };
  }
}

