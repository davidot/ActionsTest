#pragma once
#include "Piece.h"
#include "Types.h"
#include <string>
#include <string_view>
#include <utility>

namespace Chess {
    // TODO to make this actually fit in 16 bits use: struct __attribute__((packed)) Move {
    struct Move {
        enum class Flag : uint8_t {
            None = 0,
            Castling = 1,
            DoublePushPawn = 2,
            EnPassant = 3,
            PromotionToKnight = 4,
            PromotionToBishop = 5,
            PromotionToRook = 6,
            PromotionToQueen = 7
        };

        BoardIndex toPosition: 6;
        BoardIndex fromPosition : 6;

        Flag flag : 3;

        Move();

        Move(BoardIndex fromIndex, BoardIndex toIndex, Flag flags = Flag::None);

        Move(BoardIndex fromCol, BoardIndex fromRow, BoardOffset offset, Flag flags = Flag::None);

        Move(BoardIndex fromCol, BoardIndex fromRow,
             BoardIndex toCol, BoardIndex toRow, Flag flags = Flag::None);

        Move(std::string_view from, std::string_view to, Flag flags = Flag::None);

        [[nodiscard]] std::pair<BoardIndex, BoardIndex> colRowFromPosition() const;
        [[nodiscard]] std::pair<BoardIndex, BoardIndex> colRowToPosition() const;

        [[nodiscard]] bool isPromotion() const;

        [[nodiscard]] Piece::Type promotedType() const;

        bool operator==(const Move& rhs) const;
        bool operator!=(const Move& rhs) const;

        [[nodiscard]] std::string toSANSquares() const;
    };
}