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

    class Board
    {
      typedef std::array<boost::optional< std::pair<Position, PieceState> >, 64> board_type;
    public:
      Board(std::initializer_list< std::pair<Position, PieceState> > il) {
        for (const auto& p : il) {
          pieces[p.first.pos()] = p;
          if (p.second.type == Type::king) {
            if (p.second.set == Set::white) {
              whiteKing = p.first;
            } else {
              blackKing = p.first;
            }
          }
        }
      }

      class iterator : public std::iterator_traits<board_type::iterator> {
      public:
        iterator(const board_type& b, const board_type::iterator& i) : board(b), iter(i) {}
        iterator& operator ++ () {
          if (iter != board.end()) {
            for (++iter; iter != board.end() && !*iter; ++iter) {}
          }
          assert(iter == board.end() || *iter);
          return *this;
        }
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
        const_iterator& operator ++ () {
          if (iter != board.cend()) {
            for (++iter; iter != board.cend() && !*iter; ++iter) {}
          }
          assert(iter == board.end() || *iter);
          return *this;
        }
        const auto& operator * () const { return *iter; }
        bool operator == (const const_iterator& other) const { return iter == other.iter; }
        bool operator != (const const_iterator& other) const { return iter != other.iter; }
        bool operator < (const const_iterator& other) const { return iter < other.iter; }
      private:
        const board_type& board;
        board_type::const_iterator iter;
      };

      iterator begin() noexcept {
        iterator iter = { pieces, pieces.begin() };
        if (iter != end() && !*iter) {
          ++iter;
        }
        return iter;
      }
      iterator end() noexcept { return{ pieces, pieces.end() }; }
      const_iterator begin() const noexcept {
        const_iterator iter = { pieces, pieces.cbegin() };
        if (iter != end() && !*iter) {
          ++iter;
        }
        return iter;
      }
      const_iterator end() const noexcept { return{ pieces, pieces.cend() }; }

      const PieceState& operator[] (const Position& pos) const {
        return pieces[pos.pos()]->second;
      }
      void erase(const Position& pos) {
        if (pieces[pos.pos()] && pieces[pos.pos()]->second.type == Type::king) {
          if (pieces[pos.pos()]->second.set == Set::white) {
            whiteKing = BADPOS;
          }
          else {
            blackKing = BADPOS;
          }
        }
        pieces[pos.pos()].reset();
      }
      void set(const Position& pos, const PieceState& state) {
        pieces[pos.pos()] = std::make_pair(pos, state);
        if (state.type == Type::king) {
          if (state.set == Set::white) {
            whiteKing = pos;
          }
          else {
            blackKing = pos;
          }
        }
      }
      boost::optional<PieceState> get(const Position& pos) const {
        const auto& p = pieces[pos.pos()];
        return p ? p->second : boost::optional<PieceState>();
      }
      bool test(const Position& pos) const {
        return !!pieces[pos.pos()];
      }
      Position king(Set set) const { return set == Set::white ? whiteKing : blackKing; }

    private:
      Position whiteKing;
      Position blackKing;
      board_type pieces;
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
      return Position({ pos.file() + vec.df, pos.rank() + vec.dr });
    }
    inline void operator += (Position& pos, const MoveVector& vec) {
      pos = { pos.file() + vec.df, pos.rank() + vec.dr };
    }

    class ChessState
    {
    public:
      ChessState();
      
      ChessState MakeMove(const Move& move) const;

      Set activeSet;
      boost::optional<Move> lastMove;
      Board pieces;

    private:
      ChessState(Set set, const Move& move, const Board& pieces);

      void evalMoves(boost::optional<Move> xmove);
      static PieceMoves pieceMoves(const Board& pieces, const Position& pos, boost::optional<Move> xmove, const SetMoves& xmoves = SetMoves());
      static bool addMoveIf(const Board& pieces, PieceMoves& moves, const Position& pos, Set set = Set::unknown, bool capture = false);
      static bool testPath(const Board& pieces, const SetMoves& xmoves, const PiecePath& path);
      static bool testPiece(const Board& pieces, const std::pair<Position, PieceState>& piece);
    };
  }
}

