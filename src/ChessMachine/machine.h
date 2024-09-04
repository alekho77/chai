#pragma once
#include "state.h"

#include <list>

namespace Chai {
namespace Chess {
class ChessMachine : public IMachine {
 public:
    ChessMachine();
    //~ChessMachine();

    void Start() override;
    bool Move(Type type, Position from, Position to, Type promotion) override;
    bool Move(const std::string& notation) override;
    void Undo() override;

    Set CurrentPlayer() const override {
        return states.empty() ? Set::unknown : states.back().activeSet;
    }
    Pieces GetSet(Set set) const override;
    PieceMoves EnumMoves(Position from) const override;
    Status CheckStatus() const override;
    std::string LastMoveNotation() const override;

    boost::shared_ptr<IMachine> SlightClone() const override;

 private:
    ChessMachine(const ChessMachine& other);
    std::list<ChessState> states;
};
} // namespace Chess
} // namespace Chai
