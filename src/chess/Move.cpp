#include "Move.h"
#include "../util/Assertions.h"
#include "Board.h"

namespace Chess {

    Move::Move(BoardIndex fromIndex, BoardIndex toIndex, Flag flags)
        : toPosition(toIndex),
          fromPosition(fromIndex),
          flag(flags) {
    }

    Move::Move(BoardIndex fromCol, BoardIndex fromRow, BoardOffset offset, Move::Flag flags) : flag(flags) {
        BoardIndex index = Board::columnRowToIndex(fromCol, fromRow);
        toPosition = index + offset;
        fromPosition = index;
    }

    Move::Move(BoardIndex fromCol, BoardIndex fromRow, BoardIndex toCol, BoardIndex toRow, Move::Flag flags) :
       toPosition(Board::columnRowToIndex(toCol, toRow)),
       fromPosition(Board::columnRowToIndex(fromCol, fromRow)),
       flag(flags) {
    }

    Move::Move() : toPosition(0), fromPosition(0), flag(Flag::None) {
    }

    Move::Move(std::string_view from, std::string_view to, Move::Flag flags) :
        flag(flags) {
        auto fromSquare = Board::SANToIndex(from);
        auto toSquare = Board::SANToIndex(to);
        ASSERT(fromSquare);
        ASSERT(toSquare);

        fromPosition = fromSquare.value();
        toPosition = toSquare.value();
    }

    bool Move::isPromotion() const {
        return (static_cast<uint8_t>(flag) & 0x4u) != 0;
    }

    Piece::Type Move::promotedType() const {
        switch (flag) {
            case Flag::PromotionToKnight:
                return Piece::Type::Knight;
            case Flag::PromotionToBishop:
                return Piece::Type::Bishop;
            case Flag::PromotionToRook:
                return Piece::Type::Rook;
            case Flag::PromotionToQueen:
                return Piece::Type::Queen;
            default:
                break;
        }
        ASSERT_NOT_REACHED();
    }

    std::pair<BoardIndex, BoardIndex> Move::colRowFromPosition() const {
        return Board::indexToColumnRow(fromPosition);
    }

    std::pair<BoardIndex, BoardIndex> Move::colRowToPosition() const {
        return Board::indexToColumnRow(toPosition);
    }

    std::string Move::toSANSquares() const {
        if (flag == Flag::Castling) {
            std::string to = Board::indexToSAN(toPosition);
            if (to[0] == 'h') {
                to[0] = 'g';
            } else if (to[0] == 'a') {
                to[0] = 'c';
            } else {
                ASSERT_NOT_REACHED();
            }
            return Board::indexToSAN(fromPosition) + to;
        } else if (isPromotion()) {
            return Board::indexToSAN(fromPosition) + Board::indexToSAN(toPosition) + Piece{promotedType(), Color::White}.toFEN();
        } else if (fromPosition == toPosition) {
            return "-";
        }
        return Board::indexToSAN(fromPosition) + Board::indexToSAN(toPosition);
    }
}