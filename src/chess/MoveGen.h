#pragma once

#include "Board.h"
#include "Types.h"
#include "Move.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace Chess {

    class MoveList {
    public:
        size_t size() const;

        template<typename Func>
        void forEachMove(Func f) const {
            std::for_each(m_moves.begin(), m_moves.end(), f);
        }

        template<typename Predicate>
        bool hasMove(Predicate p) const {
            return std::any_of(m_moves.begin(), m_moves.end(), p);
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
        void forEachMoveFrom(BoardIndex col, BoardIndex row, Func func) const {
            forEachFilteredMove([index = Board::columnRowToIndex(col, row)](const Move& move){
                return move.fromPosition == index;
            }, func);
        }

        bool contains(Move mv) const;

        [[nodiscard]] bool isStaleMate() const;

        [[nodiscard]] bool isCheckMate() const;

        void addMove(Move);
    private:
        void kingAttacked();

        friend MoveList generateAllMoves(const Board& board);

        std::vector<Move> m_moves;
        bool m_inCheck = false;
    };

    MoveList generateAllMoves(const Board& board);

}
