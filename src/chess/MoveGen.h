#pragma once

#include "Board.h"
namespace Chess {

    class MoveList {
    public:
        size_t size() const;

        void addMove(Move);

        void addMove(Board::BoardIndex col, Board::BoardIndex row, Move::BoardOffset offset, Move::Flags flags = Move::Flags::None);
    private:
        uint32_t m_count;

    };

    MoveList generateAllMoves(const Board& board);

}
