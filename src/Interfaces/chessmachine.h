#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/container/static_vector.hpp>
#include <string>

#ifdef CHESSMACHINE_EXPORTS
#define CHESSMACHINE_API __declspec(dllexport)
#else
#define CHESSMACHINE_API __declspec(dllimport)
#endif

#ifdef CHESSENGINEGREEDY_EXPORTS
#define CHESSENGINEGREEDY_API __declspec(dllexport)
#else
#define CHESSENGINEGREEDY_API __declspec(dllimport)
#endif

#define CHESSPOS(name) const Chai::Chess::Position (##name) = { (#name)[0], (#name)[1] }
#define CHESSRANK(R) CHESSPOS(a##R); CHESSPOS(b##R); CHESSPOS(c##R); CHESSPOS(d##R); CHESSPOS(e##R); CHESSPOS(f##R); CHESSPOS(g##R); CHESSPOS(h##R)
#define CHESSBOARD CHESSRANK(1); CHESSRANK(2); CHESSRANK(3); CHESSRANK(4); CHESSRANK(5); CHESSRANK(6); CHESSRANK(7); CHESSRANK(8)

#define BADPOS Chai::Chess::Position()

namespace Chai {
  namespace Chess {
    enum class Set : char { unknown, white, black };
    enum class Type : char { bad = 0, pawn = 1, knight = 3, bishop = 4, rook = 5, queen = 9, king = 100 };
    enum class Status : char { invalid, normal, check, checkmate, stalemate };

    class Position {
    public:
      Position() : data(0xff) {}
      template <typename T> explicit Position(T p) : data(static_cast<unsigned char>(p)) {}
      Position(char f, char r) : data( (f - 'a') | ((r - '1') << 4) ) {}
      
      inline int x() const { return data & 0x0f; }
      inline int y() const { return (data >> 4) & 0x0f; }
      inline char file() const { return 'a' + x(); } // A column of the chessboard. A specific file are named using its position in 'a'–'h'.
      inline char rank() const { return '1' + y(); } // A row of the chessboard. In algebraic notation, ranks are numbered '1'–'8' starting from White's side of the board.
      inline bool operator == (const Position& other) const { return data == other.data; }
      inline bool operator != (const Position& other) const { return data != other.data; }
      inline bool operator < (const Position& other) const { return data < other.data; }
      inline bool isValid() const { return (data & 0x77) == data; }
      inline int pos() const { return (y() << 3) + x(); }
    private:
      unsigned char data;
    };
    typedef boost::container::static_vector<Position, 27> PieceMoves;

    struct Piece {
      Type type;
      Position position;
    };
    typedef boost::container::static_vector<Piece, 16> Pieces;

    class IMachine {
    public:
      virtual void Start() = 0;
      virtual bool Move(Type type, Position from, Position to, Type promotion = Type::bad) = 0;
      virtual bool Move(const std::string& notation) = 0; // Standard algebraic notation (SAN) is the notation standardized by FIDE. It omits the starting file and rank of the piece, unless it is necessary to disambiguate the move.
      virtual void Undo() = 0;

      virtual Set CurrentPlayer() const = 0;
      virtual Pieces GetSet(Set set) const = 0;
      virtual PieceMoves EnumMoves(Position from) const = 0; // Sorted vector of piece moves;
      virtual Status CheckStatus() const = 0;
      virtual std::string LastMoveNotation() const = 0;

      virtual boost::shared_ptr<IMachine> Clone() const = 0;
      
      virtual ~IMachine() {}
    };

    class IInfoCall {
    public:
      // Messages sent during the search
      virtual void NodesSearched(size_t nodes) = 0;
      virtual void NodesPerSecond(int nps) = 0;

      // Messages sent after the search
      virtual void ReadyOk() = 0;
      virtual void BestMove(std::string notation) = 0;
      virtual void BestScore(float score) = 0; // in pawns
    };

    class IEngine {
    public:
      /**
        position - the position from which to start the analysis.
        
        depth = 0 - evaluate only the current position of the current player, without further analyze moves.
        depth > 0 - evaluate the current position of the current player and further analyze half-moves (plies) in depth.
        depth < 0 - it's not used, see timeout

        timeout > 0 - limit of the estimated time in ms.
        timeout <= 0 - it's not used, you can stop the calculation by using the Stop command.
      */
      virtual bool Start(const IMachine& position, int depth, int timeout = 0) = 0;
      virtual void Stop() = 0;
      virtual void ProcessInfo(IInfoCall* cb) = 0;
      virtual float EvalPosition(const IMachine& position) const = 0; // Evaluation of the current position for current player.

      virtual ~IEngine() {}
    };
  }
}

CHESSMACHINE_API boost::shared_ptr<Chai::Chess::IMachine> CreateChessMachine();
CHESSENGINEGREEDY_API boost::shared_ptr<Chai::Chess::IEngine> CreateGreedyEngine();
