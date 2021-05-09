#pragma once

#include <cstdint>
#include <optional>
#include <ostream>
#include <string>

namespace Chess {

#define INTTYPE uint8_t

    enum class Color : INTTYPE {
        White = 0b00010000,
        Black = 0b00100000
    };

    std::ostream& operator<<(std::ostream& os, const Color& piece);

    constexpr Color opposite(Color c) {
        if (c == Color::White) {
            return Color::Black;
        }
        return Color::White;
    }

    class Piece {
    public:

        using IntType = INTTYPE;

#undef INTTYPE

        enum class Type : IntType {
            None = 0b000,
            Pawn = 0b001,
            King = 0b010,
            Bishop = 0b011,
            Rook = 0b100,
            Queen = 0b101,
            Knight = 0b110,
        };

        constexpr static IntType pieceTypes = 6;


        Piece(Type tp, Color c) noexcept;

        [[nodiscard]] IntType toInt() const;

        [[nodiscard]] char toFEN() const noexcept;

        [[nodiscard]] std::string toUTF8Char() const;

        static std::optional<Piece> fromFEN(char c);

        static std::optional<IntType> intFromFEN(char c);

        static Piece fromInt(IntType);

        bool operator==(const Piece &rhs) const;

        bool operator!=(const Piece &rhs) const;

        friend std::ostream& operator<<(std::ostream& os, const Piece& piece);

        [[nodiscard]] Color color() const;

        [[nodiscard]] Piece::Type type() const;

        [[nodiscard]] static bool isPiece(IntType val);

        [[nodiscard]] static Color colorFromInt(IntType val);

        [[nodiscard]] static Piece::Type typeFromInt(IntType val);

        [[nodiscard]] static IntType noneValue();

        [[nodiscard]] static Piece none();

    private:
        Piece(IntType val) : m_val(val) {
        }

        IntType m_val;
    };


}



