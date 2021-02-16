#include "Board.h"

namespace Chess {

    Board::Board(uint8_t size) {
    }

    Board Board::emptyBoard(uint8_t size) {
        return Board(0);
    }

    bool Board::hasValidPosition() const {
        return false;
    }

    uint32_t Board::countPieces(Piece::Color) const {
        return 0;
    }

}
