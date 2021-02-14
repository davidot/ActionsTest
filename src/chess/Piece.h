#pragma once

#include <cstdint>
#include <ostream>

namespace Chess {

    class Piece {
    public:
        enum Type : uint8_t {
            Pawn = 0u,
            Rook = 1u,
            Knight = 2u,
            Bishop = 3u,
            Queen = 4u,
            King = 5u
        };

        enum class Color : uint8_t {
            White = 0x8,
            Black = 0xF
        };

        Piece(Type tp, Color c);

        uint16_t toInt();

        char toFEN();

        static Piece fromFEN(char c);

        static uint16_t intFromFEN(char c);

        static Piece fromInt(uint16_t);

        bool operator==(const Piece &rhs) const;

        bool operator!=(const Piece &rhs) const;

        friend std::ostream &operator<<(std::ostream &os, const Piece &piece);

    private:
        Type _type;
        Color _color;
    };
}



