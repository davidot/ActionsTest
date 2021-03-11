#pragma once

#include "Board.h"
namespace Chess {

    class MoveList {
    public:
        size_t size() const;

        void addMove(Move);
    private:
        uint32_t m_count;

    };

    MoveList generateAllMoves(const Board& board);

}