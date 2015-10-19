#pragma once

#ifdef CHESSMACHINE_EXPORTS
#define CHESSMACHINE_API __declspec(dllexport)
#else
#define CHESSMACHINE_API __declspec(dllimport)
#endif

namespace Chai {
  namespace Chess {
    enum class Set : int { white, black };
    enum class Type : int { pawn = 1, knight = 3, bishop = 4, rook = 5, queen = 9, king = 1000 };

    

    class IPiece {
    public:
      virtual Set GetSet() const = 0;
      virtual Type GetType() const = 0;
      virtual char GetFile() const = 0;
      virtual char GetRank() const = 0;
      virtual ~IPiece() {}
    };

    struct Snapshot
    {
      const IPiece* whitePieces[16];
      const IPiece* blackPieces[16];
    };

    class IPosition {
    public:
      virtual const Snapshot GetSnapshot() const = 0;
      virtual ~IPosition() {}
    };

    class IChessMachine {
    public:
      virtual void Start() = 0;
      virtual const IPosition& GetPostion() const = 0;
      virtual ~IChessMachine() {}
    };

    extern "C" {
      CHESSMACHINE_API IChessMachine* CreateChessMachine();
      CHESSMACHINE_API void DeleteChessMachine(IChessMachine* ptr);
    }
  }
}