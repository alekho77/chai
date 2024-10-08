#include "machine.h"

#include <boost/regex.hpp>

namespace Chai {
namespace Chess {
ChessMachine::ChessMachine() {}

ChessMachine::ChessMachine(const ChessMachine& other) {
    if (!other.states.empty()) {
        states.push_back(other.states.back());
    }
}

void ChessMachine::Start() {
    states.clear();
    states.push_back(ChessState());
}

bool ChessMachine::Move(Type type, Position from, Position to, Type promotion) {
    if (!states.empty()) {
        const ChessState& laststate = states.back();
        const auto& piece = laststate.pieces[from];
        if (piece.valid() && piece.set == laststate.activeSet && piece.type == type) {
            if (piece.isMove(to)) {
                const char promrank = laststate.activeSet == Set::white ? '8' : '1';
                if (promotion == Type::bad) {
                    if (type == Type::pawn && to.rank() == promrank) {
                        return false; // A pawn can not come to highest rank without promotion.
                    }
                } else {
                    if (type != Type::pawn || to.rank() != promrank) {
                        return false; // Only a pawn can be promoted at the highest rank.
                    }
                    if (promotion != Type::knight && promotion != Type::bishop && promotion != Type::rook &&
                        promotion != Type::queen) {
                        return false; // Pawn can be promoted only to one of the following pieces.
                    }
                }
                states.push_back(laststate.MakeMove({type, from, to, promotion}));
                return true;
            }
        }
    }
    return false;
}

bool ChessMachine::Move(const std::string& notation) {
    if (!states.empty()) {
        boost::regex xreg("^([p,N,B,R,Q,K]?)([a-h]?)([1-8]?)(x?)([a-h])([1-8])=?([N,B,R,Q]?)");
        boost::smatch xres;
        if (boost::regex_match(notation, xres, xreg)) {
            assert(xres.size() == 8);
            std::string name = xres[1].str();
            if (name.empty()) {
                name = "p";
            }
            static const std::map<char, Type> name2type = {{'p', Type::pawn}, {'N', Type::knight}, {'B', Type::bishop},
                                                           {'R', Type::rook}, {'Q', Type::queen},  {'K', Type::king}};
            Type type = name2type.find(name.front())->second;

            std::string file1 = xres[2].str();
            std::string rank1 = xres[3].str();
            Position from = {file1.empty() ? static_cast<char>('a' + 0x0f) : file1.front(),
                             rank1.empty() ? static_cast<char>('1' + 0x0f) : rank1.front()};

            // std::string capture = xres[4].str();

            std::string file2 = xres[5].str();
            std::string rank2 = xres[6].str();
            Position to = {file2.front(), rank2.front()};

            std::string promname = xres[7].str();
            Type promotion = !promname.empty() && name2type.find(promname[0]) != name2type.end()
                               ? name2type.at(promname[0])
                               : Type::bad;

            if (!from.isValid()) {
                const ChessState& laststate = states.back();
                for (const auto& p : laststate.pieces) {
                    if (p.second.set == laststate.activeSet && p.second.type == type && p.second.isMove(to)) {
                        if (from == BADPOS) {
                            from = p.first;
                        } else if (from.x() == 0x0f) {
                            if (from.rank() == p.first.rank()) {
                                from = {p.first.file(), from.rank()};
                            }
                        } else { // from.y == 0x0f
                            if (from.file() == p.first.file()) {
                                from = {from.file(), p.first.rank()};
                            }
                        }
                    }
                    if (from.isValid()) {
                        break;
                    }
                }
            }

            if (from.isValid()) {
                return Move(type, from, to, promotion);
            }
        } else {
            const ChessState& laststate = states.back();
            const char kingrank = laststate.activeSet == Set::white ? '1' : '8';
            if (notation == "O-O") {
                return Move(Type::king, {'e', kingrank}, {'g', kingrank}, Type::bad);
            } else if (notation == "O-O-O") {
                return Move(Type::king, {'e', kingrank}, {'c', kingrank}, Type::bad);
            }
        }
    }
    return false;
}

void ChessMachine::Undo() {
    if (!states.empty()) {
        states.pop_back();
    }
}

Pieces ChessMachine::GetSet(Set set) const {
    Pieces pieces;
    if (!states.empty()) {
        const ChessState& laststate = states.back();
        for (const auto& p : laststate.pieces) {
            if (p.second.set == set) {
                pieces.push_back({p.second.type, p.first});
            }
        }
    }
    return pieces;
}

PieceMoves ChessMachine::EnumMoves(Position from) const {
    if (!states.empty()) {
        const ChessState& laststate = states.back();
        const auto& piece = laststate.pieces[from];
        if (piece.valid()) {
            return piece.moves;
        }
    }
    return PieceMoves();
}

Status ChessMachine::CheckStatus() const {
    if (!states.empty()) {
        const ChessState& laststate = states.back();
        Position king = laststate.pieces.king(laststate.activeSet);
        size_t checkcount = std::count_if(laststate.pieces.begin(), laststate.pieces.end(), [&](const auto& p) {
            return p.second.set != laststate.activeSet &&
                   std::binary_search(p.second.moves.begin(), p.second.moves.end(), king);
        });
        bool canmove = std::any_of(laststate.pieces.begin(), laststate.pieces.end(), [&](const auto& p) {
            return p.second.set == laststate.activeSet && p.second.moves.size() > 0;
        });
        if (checkcount > 0) {
            if (!canmove) {
                return Status::checkmate;
            }
            return Status::check;
        } else if (!canmove) {
            return Status::stalemate;
        }
        return Status::normal;
    }
    return Status::invalid;
}

std::string ChessMachine::LastMoveNotation() const {
    std::string lastmove;
    if (!states.empty()) {
        const ChessState& currentstate = states.back();
        auto iter = ++(states.crbegin());
        if (currentstate.lastMove && iter != states.rend()) {
            const ChessState& prevstate = *iter;
            static const std::map<Type, std::string> name = {{Type::pawn, ""},    {Type::knight, "N"},
                                                             {Type::bishop, "B"}, {Type::rook, "R"},
                                                             {Type::queen, "Q"},  {Type::king, "K"}};
            if (currentstate.lastMove->type == Type::king) {
                const char kingrank = prevstate.activeSet == Set::white ? '1' : '8';
                if (currentstate.lastMove->from.rank() == kingrank && currentstate.lastMove->to.rank() == kingrank &&
                    currentstate.lastMove->from.file() == 'e') {
                    if (currentstate.lastMove->to.file() == 'g') {
                        return "O-O";
                    } else if (currentstate.lastMove->to.file() == 'c') {
                        return "O-O-O";
                    }
                }
            }
            lastmove = name.at(currentstate.lastMove->type);
            if (currentstate.lastMove->type == Type::pawn) {
                if (currentstate.lastMove->from.file() != currentstate.lastMove->to.file()) {
                    lastmove = currentstate.lastMove->from.file();
                }
            } else if (std::count_if(prevstate.pieces.begin(), prevstate.pieces.end(), [&](const auto& p) {
                           return p.second.set == prevstate.activeSet && p.second.type == currentstate.lastMove->type &&
                                  std::binary_search(p.second.moves.begin(), p.second.moves.end(),
                                                     currentstate.lastMove->to);
                       }) > 1) {
                // More than one piece could make this move.
                size_t files = std::count_if(prevstate.pieces.begin(), prevstate.pieces.end(), [&](const auto& p) {
                    return p.second.set == prevstate.activeSet && p.second.type == currentstate.lastMove->type &&
                           p.first.file() == currentstate.lastMove->from.file() &&
                           std::binary_search(p.second.moves.begin(), p.second.moves.end(), currentstate.lastMove->to);
                });
                assert(files >= 1);
                size_t ranks = std::count_if(prevstate.pieces.begin(), prevstate.pieces.end(), [&](const auto& p) {
                    return p.second.set == prevstate.activeSet && p.second.type == currentstate.lastMove->type &&
                           p.first.rank() == currentstate.lastMove->from.rank() &&
                           std::binary_search(p.second.moves.begin(), p.second.moves.end(), currentstate.lastMove->to);
                });
                assert(ranks >= 1);
                if (files > 1) {
                    if (ranks > 1) {
                        lastmove += currentstate.lastMove->from.file();
                        lastmove += currentstate.lastMove->from.rank();
                    } else {
                        lastmove += currentstate.lastMove->from.rank();
                    }
                } else {
                    lastmove += currentstate.lastMove->from.file();
                }
            }
            if ((prevstate.pieces.test(currentstate.lastMove->to)) ||
                (currentstate.lastMove->type == Type::pawn &&
                 currentstate.lastMove->from.file() != currentstate.lastMove->to.file())) {
                lastmove += "x";
            }
            lastmove += currentstate.lastMove->to.file();
            lastmove += currentstate.lastMove->to.rank();
            if (currentstate.lastMove->promotion != Type::bad) {
                lastmove += "=" + name.at(currentstate.lastMove->promotion);
            }
        }
    }
    return lastmove;
}

boost::shared_ptr<IMachine> ChessMachine::SlightClone() const {
    return boost::shared_ptr<IMachine>(new ChessMachine(*this));
}

} // namespace Chess
} // namespace Chai