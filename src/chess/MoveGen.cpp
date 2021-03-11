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
                if (!opt_piece) {
                    continue;
                }
                switch (opt_piece->type()) {
                    case Piece::Type::Pawn:
                        add_move(col, row, col, row + 1);
                        break;
                    case Piece::Type::King:
                        break;
                    case Piece::Type::Knight:
                        break;
                    case Piece::Type::Bishop:
                        break;
                    case Piece::Type::Rook:
                        break;
                    case Piece::Type::Queen:
                        break;
                }
            }
        }
        return list;
    }

    void MoveList::addMove(Move) {
        m_count++;
    }
}
