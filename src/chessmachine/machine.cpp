#include "stdafx.h"
#include "machine.h"

namespace Chai {
  namespace Chess {
    ChessMachine::ChessMachine() {

    }

    void ChessMachine::Start() {
      states.clear();
      states.push_back(ChessState());
    }

    bool ChessMachine::Move(Type type, Position from, Position to)
    {
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        auto piece = laststate.pieces.find(from);
        if (piece != laststate.pieces.end() && piece->second.set == laststate.activeSet && piece->second.type == type) {
          assert(piece->second.moves.find(to) != piece->second.moves.end());
          const auto& move = piece->second.moves.find(to);
          if (move != piece->second.moves.end()) {
            states.push_back(ChessState(laststate, { type, from, to }));
            return true;
          }
        }
      }
      return false;
    }

    bool ChessMachine::Move(const char* notation)
    {
      if (!states.empty()) {
        boost::regex xreg("^([p,N,B,R,Q,K]?)([a-h]?)([1-8]?)(x?)([a-h])([1-8])([N,B,R,Q,K]?)");
        boost::smatch xres;
        std::string snotation(notation);
        if (boost::regex_match(snotation, xres, xreg)) {
          assert(xres.size() == 8);
          std::string name = xres[1].str();
          if (name.empty()) {
            name = "p";
          }
          static const std::map<char, Type> name2type = { {'p',Type::pawn}, {'N',Type::knight}, {'B',Type::bishop}, {'R',Type::rook}, {'Q',Type::queen}, {'K',Type::king} };
          Type type = name2type.find(name.front())->second;

          std::string file1 = xres[2].str();
          std::string rank1 = xres[3].str();
          Position from = { file1.empty() ? 0 : file1.front(), rank1.empty() ? 0 : rank1.front() };

          //std::string capture = xres[4].str();

          std::string file2 = xres[5].str();
          std::string rank2 = xres[6].str();
          Position to = { file2.front(), rank2.front() };

          //std::string promotion = xres[7].str();

          if (!from.isValid()) {
            const ChessState& laststate = states.back();
            for (auto p : laststate.pieces) {
              if (p.second.set == laststate.activeSet && p.second.type == type && p.second.moves.find(to) != p.second.moves.end()) {
                if (from == BADPOS) {
                  from = p.first;
                } else if (from.file == 0) {
                  if (from.rank == p.first.rank) {
                    from.file = p.first.file;
                  }
                } else { // from.rank == 0
                  if (from.file == p.first.file) {
                    from.rank = p.first.rank;
                  }
                }
              }
              if (from.isValid()) {
                break;
              }
            }
          }
          
          if (from.isValid()) {
            return Move(type, from, to);
          }
        }
      }
      return false;
    }

    const Piece* ChessMachine::GetSet(Set set) const
    {
      piecesSet.clear();
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        piecesSet.reserve(laststate.pieces.size() + 1);
        for (auto p : laststate.pieces) {
          if (p.second.set == set) {
            piecesSet.push_back({ p.second.type, p.first });
          }
        }
        piecesSet.push_back({ Type::bad, BADPOS });
        return &piecesSet[0];
      }
      return nullptr;
    }

    const Position* ChessMachine::CheckMoves(Position from) const
    {
      pieceMoves.clear();
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        auto piece = laststate.pieces.find(from);
        if (piece != laststate.pieces.end()) {
          for (const Position& m : piece->second.moves) {
            pieceMoves.push_back(m);
          }
          pieceMoves.push_back(BADPOS);
          return &pieceMoves[0];
        }
      }
      return nullptr;
    }

    Status ChessMachine::CheckStatus() const
    {
      if (!states.empty()) {
        const ChessState& laststate = states.back();
        Position king = std::find_if(laststate.pieces.begin(), laststate.pieces.end(), [&](const auto& p) { return p.second.set == laststate.activeSet && p.second.type == Type::king; })->first;
        size_t checkcount = std::count_if(laststate.pieces.begin(), laststate.pieces.end(), [&](const auto& p) { return p.second.set != laststate.activeSet && p.second.moves.find(king) != p.second.moves.end(); });
        bool canmove = std::any_of(laststate.pieces.begin(), laststate.pieces.end(), [&](const auto& p) { return p.second.set == laststate.activeSet && p.second.moves.size() > 0; });
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

  }
}