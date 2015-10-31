#pragma once

#ifdef CHESSMACHINE_EXPORTS
#define CHESSMACHINE_API __declspec(dllexport)
#else
#define CHESSMACHINE_API __declspec(dllimport)
#endif

#define CHESSPOS(name) const Postion (##name) = { (#name)[0], (#name)[1] }
#define CHESSRANK(R) CHESSPOS(a##R); CHESSPOS(b##R); CHESSPOS(c##R); CHESSPOS(d##R); CHESSPOS(e##R); CHESSPOS(f##R); CHESSPOS(g##R); CHESSPOS(h##R)
#define CHESSBOARD CHESSRANK(1); CHESSRANK(2); CHESSRANK(3); CHESSRANK(4); CHESSRANK(5); CHESSRANK(6); CHESSRANK(7); CHESSRANK(8)

#define BADPOS Postion({0,0})

namespace Chai {
  namespace Chess {
    enum class Set : int { unknown, white, black };
    enum class Type : int { bad = 0, pawn = 1, knight = 3, bishop = 4, rook = 5, queen = 9, king = 1000 };

    struct Postion
    {
      char file; // A column of the chessboard. A specific file are named using its position in 'a'–'h'.
      char rank; // A row of the chessboard. In algebraic notation, ranks are numbered '1'–'8' starting from White's side of the board.
      bool operator == (const Postion& other) const { return file == other.file && rank == other.rank; }
      bool operator != (const Postion& other) const { return file != other.file || rank != other.rank; }
      bool operator < (const Postion& other) const { return file < other.file || (file == other.file && rank < other.rank); }
      bool isValid() const { return file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8'; }
    };

    struct SetPieces
    {
      struct Piece
      {
        Type type;
        Postion position;
      };
      int count;
      Piece pieces[16];
    };

    struct PieceMoves
    {
      struct Move
      {
        bool threat;
        Postion move;
      };
      int count;
      Move moves[27];
    };

    class IChessMachine {
    public:
      virtual void Start() = 0;
      virtual bool Move(Type type, Postion from, Postion to) = 0;
      virtual bool Move(const char* notation) = 0; // Standard algebraic notation (SAN) is the notation standardized by FIDE. It omits the starting file and rank of the piece, unless it is necessary to disambiguate the move.

      virtual SetPieces GetSet(Set set) const = 0;
      virtual PieceMoves CheckMoves(Postion from) const = 0;

      virtual ~IChessMachine() {}
    };

    CHESSMACHINE_API IChessMachine* CreateChessMachine();
    CHESSMACHINE_API void DeleteChessMachine(IChessMachine* ptr);
  }
}