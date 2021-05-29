#include "Move.h"
#include "../util/Assertions.h"
#include "Board.h"
#include <optional>
#include <type_traits>

namespace Chess {

    Move::Move(BoardIndex fromIndex, BoardIndex toIndex, Flag flags)
        : toPosition(toIndex),
          fromPosition(fromIndex),
          flag(flags) {
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
        return (static_cast<std::underlying_type_t<Flag>>(flag) & 0x4u) != 0;
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

    Move Move::fromSANSquares(std::string_view vw, const Board& board) {
        Flag flag = Flag::None;
        if (vw.size() > 4) {
            ASSERT(vw.size() == 5);
            ASSERT(Piece::fromFEN(vw[4]).has_value());
            Piece::Type promoType = Piece::fromFEN(vw[4])->type();
            flag = promotionFromType(promoType);
            vw.remove_suffix(1);
        }
        ASSERT(vw.size() == 4);
        ASSERT(Board::SANToIndex(vw.substr(0, 2)).has_value());
        ASSERT(Board::SANToIndex(vw.substr(2, 2)).has_value());
        BoardIndex from = *Board::SANToIndex(vw.substr(0, 2));
        BoardIndex to = *Board::SANToIndex(vw.substr(2, 2));

        ASSERT(board.pieceAt(from).has_value());
        auto p = board.pieceAt(from);
        ASSERT(p->color() == board.colorToMove());

        // TODO: parse move correctly
        if (p->type() == Piece::Type::King) {
            // check castle
        } else if (p->type() == Piece::Type::Pawn) {
            // check double push & en passant
        }

        // TODO: do we want this function?
        ASSERT_NOT_REACHED();

        return {from, to};
    }

    Move::Flag Move::promotionFromType(Piece::Type tp) {
        switch (tp) {
            case Piece::Type::Queen:
                return Flag::PromotionToQueen;
            case Piece::Type::Knight:
                return Flag::PromotionToKnight;
            case Piece::Type::Bishop:
                return Flag::PromotionToBishop;
            case Piece::Type::Rook:
                return Flag::PromotionToRook;
            default:
                break;
        }
        ASSERT_NOT_REACHED();
    }

    bool Move::operator==(const Move& rhs) const {
        return toPosition == rhs.toPosition &&
               fromPosition == rhs.fromPosition &&
               flag == rhs.flag;
    }

    bool Move::operator!=(const Move& rhs) const {
        return !(rhs == *this);
    }
}