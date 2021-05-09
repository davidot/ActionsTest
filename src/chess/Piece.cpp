#include "Piece.h"
#include "../util/Assertions.h"

#include <algorithm>
#include <iterator>
#include <string_view>

namespace Chess {

#define ENUM_TO_INT(val) static_cast<Piece::IntType>(val)
    constexpr Piece::IntType typeMask = 0b1111;
    constexpr Piece::IntType whiteMask = ENUM_TO_INT(Color::White);
    constexpr Piece::IntType blackMask = ENUM_TO_INT(Color::Black);

    constexpr Piece::IntType colorMask = whiteMask | blackMask;

    Piece::Piece(Piece::Type tp, Color c) noexcept {
        m_val = ENUM_TO_INT(tp)
               | ENUM_TO_INT(c);
    }

    Piece::IntType Piece::toInt() const {
        return m_val;
    }

    Piece::Type Piece::type() const {
        return static_cast<Piece::Type>(m_val & typeMask);
    }

    Color Piece::color() const {
        return static_cast<Color>(m_val & colorMask);
    }

    constexpr const auto caseDiff = 'a' - 'A';

    char Piece::toFEN() const noexcept {
        char c;
        switch (type()) {
            case Type::Pawn:
                c = 'P';
                break;
            case Type::Rook:
                c = 'R';
                break;
            case Type::Knight:
                c = 'N';
                break;
            case Type::Bishop:
                c = 'B';
                break;
            case Type::Queen:
                c = 'Q';
                break;
            case Type::King:
                c = 'K';
                break;
            default:
                ASSERT_NOT_REACHED();
        }
        if (m_val & blackMask) {
            c += caseDiff;
        }
        return c;
    }

    Piece::Type typeFromFEN(char c) {
        using T = Piece::Type;
        switch (c) {
            case 'P':
                return T::Pawn;
            case 'R':
                return T::Rook;
            case 'N':
                return T::Knight;
            case 'B':
                return T::Bishop;
            case 'Q':
                return T::Queen;
            case 'K':
                return T::King;
            default:
                break;
        }
        ASSERT_NOT_REACHED();
    }

    // Just hardcode for now...
    constexpr std::string_view allValues = "PRNBQK";

    std::optional<Piece> Piece::fromFEN(char c) {
        // This does assume ascii...

        bool lower = c >= 'a';
        Color col = lower ? Color::Black : Color::White;
        if (lower) {
            c -= caseDiff;
        }
        if (std::find(allValues.begin(), allValues.end(), c) == allValues.end()) {
            return std::nullopt;
        }
        return Piece(typeFromFEN(c), col);
    }

    std::optional<Piece::IntType> Piece::intFromFEN(char c) {
        bool lower = c >= 'a';
        if (lower) {
            c -= caseDiff;
        }
        if (std::find(allValues.begin(), allValues.end(), c) == allValues.end()) {
            return std::nullopt;
        }
        return ENUM_TO_INT(typeFromFEN(c)) | ENUM_TO_INT(lower ? Color::Black : Color::White);
    }

    Piece Piece::fromInt(IntType i) {
        auto c = (i & whiteMask) != 0 ? Color::White : Color::Black;
        return Piece(static_cast<Piece::Type>(i & typeMask), c);
    }

    bool Piece::operator==(const Piece &rhs) const {
        return m_val == rhs.m_val;
    }

    bool Piece::operator!=(const Piece &rhs) const {
        return !(rhs == *this);
    }

    const char* colorName(Color c) {
        switch (c) {
            case Color::White:
                return "White";
            case Color::Black:
                return "Black";
        }
        ASSERT_NOT_REACHED();
    }

    const char* pieceName(Piece::Type t) {
        switch (t) {
            case Piece::Type::Pawn:
                return "Pawn";
            case Piece::Type::Rook:
                return "Rook";
            case Piece::Type::Knight:
                return "Knight";
            case Piece::Type::Bishop:
                return "Bishop";
            case Piece::Type::Queen:
                return "Queen";
            case Piece::Type::King:
                return "King";
            default:
                break;
        }
        ASSERT_NOT_REACHED();
    }

    std::ostream& operator<<(std::ostream& os, const Piece& piece) {
        if (piece.m_val == Piece::noneValue()) {
            os << "No piece";
        } else {
            os << colorName(piece.color()) << ' ' << pieceName(piece.type());
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Color& color) {
        os << colorName(color);
        return os;
    }

    bool Piece::isPiece(Piece::IntType val) {
        return ((val & whiteMask) ^ ((val & blackMask) >> 1)) != 0;
    }

    Piece Piece::none() {
        return Piece(0);
    }

    Piece::IntType Piece::noneValue() {
        return 0;
    }

    Color Piece::colorFromInt(Piece::IntType val) {
        ASSERT(isPiece(val));
        switch (val & colorMask) {
            case whiteMask:
                return Color::White;
            case blackMask:
                return Color::Black;
        }
        ASSERT_NOT_REACHED();
    }

    Piece::Type Piece::typeFromInt(uint8_t val) {
        return static_cast<Piece::Type>(val & typeMask);
    }


    std::string Piece::toUTF8Char() const {
        static std::string mappings[] = {
                "", // empty
                "♟",
                "♚",
                "♞",
                "♝",
                "♜",
                "♛",
                "", // unused
                "", // empty
                "♙",
                "♔",
                "♘",
                "♗",
                "♖",
                "♕",
                "", // unused
        };

        size_t i = (color() == Color::Black ? 0u : 8u) + ENUM_TO_INT(type());
        ASSERT(i < std::size(mappings));
        return mappings[i];
    }

}
