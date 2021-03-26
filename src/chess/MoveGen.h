#pragma once

#include "Board.h"
#include <algorithm>

namespace Chess {

    class MoveList {
    public:
        size_t size() const;

        void addMove(Move);

        void addMove(Board::BoardIndex col, Board::BoardIndex row, Move::BoardOffset offset, Move::Flag flags = Move::Flag::None);

        template<typename Func>
        void forEachMove(Func f) const {
            std::for_each(m_moves.begin(), m_moves.end(), f);
        }

        template<typename Filter, typename Func>
        void forEachFilteredMove(Filter filter, Func func) const {
            forEachMove([&](const auto& mv){
              if (filter(mv)) {
                  func(mv);
              }
            });
        }

        template<typename Func>
        void forEachMoveFrom(Board::BoardIndex col, Board::BoardIndex row, Func func) const {
            forEachFilteredMove([index = Board::columnRowToIndex(col, row)](const Move& move){
                return move.fromPosition == index;
            }, func);
        }
    private:
        std::vector<Move> m_moves;
    };

    MoveList generateAllMoves(const Board& board);

}
