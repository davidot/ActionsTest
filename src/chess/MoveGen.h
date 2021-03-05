#pragma once

#include "Board.h"
namespace Chess {

    class MoveList {
    public:
        size_t size();
    };

    MoveList generateAllMoves(const Board& board);

}