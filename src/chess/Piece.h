#pragma once

#include <cstdint>
#include <ostream>

namespace Chess {

    class Piece {
    public:

        using IntType = uint8_t;

        enum class Type : IntType {
            Pawn = 0b0000,
            King = 0b0011,
            Knight = 0b1000,
            Bishop = 0b0101,
            Rook = 0b0110,
            Queen = 0b0111,
        };

        enum class Color : IntType {
            White = 0b00010000,
            Black = 0b00100000
        };

        Piece(Type tp, Color c);

        [[nodiscard]] IntType toInt() const;

        [[nodiscard]] char toFEN() const;

        [[nodiscard]] bool isPawn() const;
        [[nodiscard]] bool canKnightJump() const;
        [[nodiscard]] bool canMoveDiagonally() const;
        [[nodiscard]] bool canMoveAxisAligned() const;
        [[nodiscard]] bool canMoveUnlimited() const;

        static Piece fromFEN(char c);

        static IntType intFromFEN(char c);

        static Piece fromInt(IntType);

        bool operator==(const Piece &rhs) const;

        bool operator!=(const Piece &rhs) const;

        friend std::ostream &operator<<(std::ostream &os, const Piece &piece);

    private:
        IntType m_val;

        [[nodiscard]] Piece::Type type() const;
        [[nodiscard]] Piece::Color color() const;
    };
}



