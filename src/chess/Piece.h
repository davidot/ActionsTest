#pragma once

#include <cstdint>
#include <ostream>

namespace Chess {

    class Piece {
    public:

        using IntType = uint8_t;

        enum Type : IntType {
            Pawn = 0u,
            Rook = 1u,
            Knight = 2u,
            Bishop = 3u,
            Queen = 4u,
            King = 5u
        };

        enum class Color : IntType {
            White = 0x8,
            Black = 0x10
        };

        Piece(Type tp, Color c);

        IntType toInt();

        char toFEN();

        static Piece fromFEN(char c);

        static IntType intFromFEN(char c);

        static Piece fromInt(IntType);

        bool operator==(const Piece &rhs) const;

        bool operator!=(const Piece &rhs) const;

        friend std::ostream &operator<<(std::ostream &os, const Piece &piece);

    private:
        Type _type;
        Color _color;
    };
}



