#pragma once

#ifdef CHESSMACHINE_EXPORTS
#define CHESSMACHINE_API __declspec(dllexport)
#else
#define CHESSMACHINE_API __declspec(dllimport)
#endif

namespace Chai {
  namespace Chess {
    //enum class Set : int { white, black };
    enum class Type : int { bad = 0, pawn = 1, knight = 3, bishop = 4, rook = 5, queen = 9, king = 1000 };

    struct Snapshot
    {
      struct Postion
      {
        char file; // A column of the chessboard. A specific file are named using its position in 'a'–'h'.
        char rank; // A row of the chessboard. In algebraic notation, ranks are numbered '1'–'8' starting from White's side of the board.
      };
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