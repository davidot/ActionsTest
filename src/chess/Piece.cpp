#include "Piece.h"
#include <cassert>

namespace Chess {
    Piece::Piece(Piece::Type tp, Piece::Color c) : _type(tp), _color(c) {
    }

    Piece::IntType Piece::toInt() {
        return static_cast<IntType>(_type)
               | static_cast<IntType>(_color);
    }

    constexpr const auto caseDiff = 'a' - 'A';

    char Piece::toFEN() {
        char c;
        switch (_type) {
            case Pawn:
                c = 'p';
                break;
            case Rook:
                c = 'r';
                break;
            case Knight:
                c = 'n';
                break;
            case Bishop:
                c = 'b';
                break;
            case Queen:
                c = 'q';
                break;
            case King:
                c = 'k';
                break;
            default:
                c = '?';
        }
        if (_color == Color::White) {
            c -= caseDiff;
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
                return T::Pawn;
        }
    }

    Piece Piece::fromFEN(char c) {
        // This does assume ascii...
        bool lower = c >= 'a';
        Piece::Color col = lower ? Color::Black : Color::White;
        if (lower) {
            c -= caseDiff;
        }
        return Piece(typeFromFEN(c), col);
    }

    Piece::IntType Piece::intFromFEN(char c) {
        bool lower = c >= 'a';
        if (lower) {
            c -= caseDiff;
        }
        return static_cast<IntType>(typeFromFEN(c)) | static_cast<IntType>(lower ? Color::Black : Color::White);
    }

    Piece Piece::fromInt(IntType i) {
        constexpr auto wVal = static_cast<uint16_t>(Piece::Color::White);
        auto c = (i & wVal) != 0 ? Piece::Color::White : Piece::Color::Black;
        return Piece(static_cast<Piece::Type>(i & 0b111), c);

    }

    bool Piece::operator==(const Piece &rhs) const {
        return _type == rhs._type &&
               _color == rhs._color;
    }

    bool Piece::operator!=(const Piece &rhs) const {
        return !(rhs == *this);
    }

    const char* colorName(Piece::Color c) {
        switch (c) {
            case Piece::Color::White:
                return "White";
            case Piece::Color::Black:
                return "Black";
        }
        return "?";
    }

    const char* pieceName(Piece::Type t) {
        switch (t) {
            case Piece::Pawn:
                return "Pawn";
            case Piece::Rook:
                return "Rook";
            case Piece::Knight:
                return "Knight";
            case Piece::Bishop:
                return "Bishop";
            case Piece::Queen:
                return "Queen";
            case Piece::King:
                return "King";
        }
        return "?";
    }

    std::ostream &operator<<(std::ostream &os, const Piece &piece) {
        os << colorName(piece._color) << ' ' << pieceName(piece._type);
        return os;
    }
}
