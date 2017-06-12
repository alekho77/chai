#pragma once

#include "chessmachine.h"

#include <boost/regex.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <algorithm>
#include <map>
#include <vector>

namespace Chai {
namespace Chess {

typedef std::map< Position, PieceMoves > TestMoves;

template <class T>
bool contains(const T& list, Type type) {
  return std::find_if(list.begin(), list.end(), [type](const auto& p) { return p.type == type; }) != list.end();
}

template <class T>
typename size_t count(const T& list, Type type) {
  return std::count_if(list.begin(), list.end(), [type](const auto& p) { return p.type == type; });
}

template <class T>
typename typename T::const_iterator at(const T& list, Position pos) {
  return std::find_if(list.begin(), list.end(), [pos](const auto& p) { return p.position == pos; });
}

template <class T>
bool exactly(const T& list, Position pos, Type type) {
  auto p = at(list, pos);
  if (p != list.end()) {
    return p->type == type;
  }
  return false;
}

template <class T>
bool equal(const T& list1, const T& list2) {
  T tmp1 = list1;
  T tmp2 = list2;
  std::sort(tmp1.begin(), tmp1.end());
  std::sort(tmp2.begin(), tmp2.end());
  return tmp1 == tmp2;
}

inline std::vector<std::string> split(const std::string& game) {
  std::vector<std::string> moves;
  boost::regex xreg("(?|(\\d+)\\.(?|([p,N,B,R,Q,K,1-8,a-h,x,=]+)|(O-O-O)|(O-O))\\+*\\s+(?|([p,N,B,R,Q,K,1-8,a-h,x,=]+)|(O-O-O)|(O-O))|(\\d+)\\.(?|([p,N,B,R,Q,K,1-8,a-h,x,=]+)|(O-O-O)|(O-O)))");
  for (auto xit = make_regex_iterator(game, xreg); xit != boost::sregex_iterator(); ++xit) {
    auto& res = *xit;
    assert(res.size() == 4);
    int nm = std::stoi(res[1].str());
    assert(nm == (moves.size() / 2 + 1));
    moves.push_back(res[2].str());
    std::string bm = res[3].str();
    if (!bm.empty()) {
      moves.push_back(bm);
    }
  }
  return moves;
}

inline std::string toStr(const Position& p) {
  return p.isValid() ? std::string(1, p.file()) + std::string(1, p.rank()) : std::string("");
}

inline void testpos(const std::map<Type, TestMoves>& position, const Pieces& pieces, const IMachine& machine) {
  static const std::map<Type, std::string> name = { { Type::pawn, "p" },{ Type::knight, "N" },{ Type::bishop, "B" },{ Type::rook, "R" },{ Type::queen, "Q" },{ Type::king, "K" } };
  for (const auto& p : position) {
    BOOST_CHECK_MESSAGE(p.second.size() == count(pieces, p.first), "The number of pieces " + name.at(p.first) + " does not match");
    for (const auto& m : p.second) {
      BOOST_CHECK_MESSAGE(exactly(pieces, m.first, p.first), "The piece " + name.at(p.first) + " was not found at the position \"" + toStr(m.first) + "\"");
      BOOST_CHECK_MESSAGE(equal(machine.EnumMoves(m.first), m.second), "Moves list does not match for piece " + name.at(p.first) + " at the position \"" + toStr(m.first) + "\"");
    }
  }
}
}
}
