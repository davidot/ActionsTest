#include "MoveGen.h"


namespace Chess {

    size_t MoveList::size() const {
        return m_count;
    }

    MoveList generateAllMoves(const Board &board) {
        using BI = Board::BoardIndex;

        MoveList list{};
        auto add_move = [&](BI fromCol, BI fromRow,
                            BI toCol, BI toRow) {
            list.addMove(Move{});
        };

        for (BI col = 0; col < board.size(); col++) {
            for (BI row = 0; row < board.size(); row++) {
                auto opt_piece = board.pieceAt(col, row);
                if (opt_piece && opt_piece->isPawn()) {
                    add_move(col, row, col, row + 1);
                }
            }
        }
        return list;
    }

    void MoveList::addMove(Move) {
        m_count++;
    }
}
