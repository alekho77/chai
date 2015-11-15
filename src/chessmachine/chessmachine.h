#pragma once

#ifdef CHESSMACHINE_EXPORTS
#define CHESSMACHINE_API __declspec(dllexport)
#else
#define CHESSMACHINE_API __declspec(dllimport)
#endif

#define CHESSPOS(name) const Position (##name) = { (#name)[0], (#name)[1] }
#define CHESSRANK(R) CHESSPOS(a##R); CHESSPOS(b##R); CHESSPOS(c##R); CHESSPOS(d##R); CHESSPOS(e##R); CHESSPOS(f##R); CHESSPOS(g##R); CHESSPOS(h##R)
#define CHESSBOARD CHESSRANK(1); CHESSRANK(2); CHESSRANK(3); CHESSRANK(4); CHESSRANK(5); CHESSRANK(6); CHESSRANK(7); CHESSRANK(8)

#define BADPOS Chai::Chess::Position({0,0})

namespace Chai {
  namespace Chess {
    enum class Set : int { unknown, white, black };
    enum class Type : int { bad = 0, pawn = 1, knight = 3, bishop = 4, rook = 5, queen = 9, king = 1000 };
    enum class Status : int { invalid, normal, check, checkmate, stalemate };

    struct Position {
      char file; // A column of the chessboard. A specific file are named using its position in 'a'–'h'.
      char rank; // A row of the chessboard. In algebraic notation, ranks are numbered '1'–'8' starting from White's side of the board.
      bool operator == (const Position& other) const { return file == other.file && rank == other.rank; }
      bool operator != (const Position& other) const { return file != other.file || rank != other.rank; }
      bool operator < (const Position& other) const { return file < other.file || (file == other.file && rank < other.rank); }
      bool isValid() const { return file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8'; }
    };

    struct Piece {
      Type type;
      Position position;
    };

    class IMachine {
    public:
      virtual void Start() = 0;
      virtual bool Move(Type type, Position from, Position to, Type promotion = Type::bad) = 0;
      virtual bool Move(const char* notation) = 0; // Standard algebraic notation (SAN) is the notation standardized by FIDE. It omits the starting file and rank of the piece, unless it is necessary to disambiguate the move.
      virtual void Undo() = 0;

      virtual Set CurrentMove() const = 0;
      virtual const Piece* GetSet(Set set) const = 0; // Type::bad type terminated array or nullptr if it is not started.
      virtual const Position* CheckMoves(Position from) const = 0; // BADPOS terminated array or nullptr if move is impossible.
      virtual Status CheckStatus() const = 0;
      virtual const char* LastMoveNotation() const = 0;

      virtual IMachine* Clone() const = 0;
      
      virtual ~IMachine() {}
    };

    CHESSMACHINE_API IMachine* CreateChessMachine();
    CHESSMACHINE_API void DeleteChessMachine(IMachine* ptr);

    class IInfoCall {
    public:
      // Messages sent during the search
      virtual void SearchDepth(int depth) = 0;
      virtual void NodesSearched(size_t nodes) = 0;
      virtual void NodesPerSecond(int nps) = 0;

      // Messages sent after the search
      virtual void ReadyOk() = 0;
      virtual void BestMove(const char* notation) = 0;
      virtual void BestScore(int score) = 0;
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

      virtual ~IEngine() {}
    };
  }
}