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
    enum class Set : int { white, black };
    enum class Type : int { bad = 0, pawn = 1, knight = 3, bishop = 4, rook = 5, queen = 9, king = 1000 };

    struct Postion
    {
      char file; // A column of the chessboard. A specific file are named using its position in 'a'–'h'.
      char rank; // A row of the chessboard. In algebraic notation, ranks are numbered '1'–'8' starting from White's side of the board.
      bool operator == (const Postion& other) const { return this->file == other.file && this->rank == other.rank; }
    };

    struct Snapshot
    {
      struct Field
      {
        bool threat;
        Postion postion;
      };
      struct Piece
      {
        Type type;
        Postion position;
        Field moves[27];
      };

      Piece whitePieces[16];
      Piece blackPieces[16];
    };

    class IChessMachine {
    public:
      virtual void Start() = 0;
      virtual bool GetSnapshot(Snapshot& snapshot) const = 0;
      virtual ~IChessMachine() {}
    };

    CHESSMACHINE_API IChessMachine* CreateChessMachine();
    CHESSMACHINE_API void DeleteChessMachine(IChessMachine* ptr);
  }
}