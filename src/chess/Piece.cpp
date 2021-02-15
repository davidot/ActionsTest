#include "Piece.h"

namespace Chess {

#define ENUM_TO_INT(val) static_cast<Piece::IntType>(val)

    constexpr Piece::IntType typeMask = 0b1111;
    constexpr Piece::IntType whiteMask = ENUM_TO_INT(Piece::Color::White);
    constexpr Piece::IntType colorMask = whiteMask | ENUM_TO_INT(Piece::Color::Black);

    Piece::Piece(Piece::Type tp, Piece::Color c) {
        _val = ENUM_TO_INT(tp)
               | ENUM_TO_INT(c);
    }

    Piece::IntType Piece::toInt() const {
        return _val;
    }

    Piece::Type Piece::type() const {
        return static_cast<Piece::Type>(_val & typeMask);
    }

    Piece::Color Piece::color() const {
        return static_cast<Piece::Color>(_val & colorMask);
    }

    constexpr const auto caseDiff = 'a' - 'A';

    char Piece::toFEN() const {
        char c;
        switch (type()) {
            case Type::Pawn:
                c = 'p';
                break;
            case Type::Rook:
                c = 'r';
                break;
            case Type::Knight:
                c = 'n';
                break;
            case Type::Bishop:
                c = 'b';
                break;
            case Type::Queen:
                c = 'q';
                break;
            case Type::King:
                c = 'k';
                break;
            default:
                c = '?';
        }
        if (_val & whiteMask) {
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
        return ENUM_TO_INT(typeFromFEN(c)) | ENUM_TO_INT(lower ? Color::Black : Color::White);
    }

    Piece Piece::fromInt(IntType i) {
        auto c = (i & whiteMask) != 0 ? Piece::Color::White : Piece::Color::Black;
        return Piece(static_cast<Piece::Type>(i & typeMask), c);

    }

    bool Piece::operator==(const Piece &rhs) const {
        return _val == rhs._val;
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
        }
        return "?";
    }

    std::ostream &operator<<(std::ostream &os, const Piece &piece) {
        os << colorName(piece.color()) << ' ' << pieceName(piece.type());
        return os;
    }

    bool Piece::isPawn() const {
        return (_val & typeMask) == ENUM_TO_INT(Piece::Type::Pawn);
    }
    bool Piece::canKnightJump() const {
        constexpr const auto knightMask = ENUM_TO_INT(Piece::Type::Knight);
        return (_val & typeMask) == knightMask;
    }
    bool Piece::canMoveDiagonally() const {
        constexpr const IntType diagMask = ENUM_TO_INT(Piece::Type::Bishop) & ENUM_TO_INT(Piece::Type::Queen) & ENUM_TO_INT(Piece::Type::King);
        return (_val & typeMask & diagMask) != 0;
    }
    bool Piece::canMoveAxisAligned() const {
        constexpr const IntType axisMask = ENUM_TO_INT(Piece::Type::Rook) & ENUM_TO_INT(Piece::Type::Queen) & ENUM_TO_INT(Piece::Type::King);
        return (_val & typeMask & axisMask) != 0;
    }
    bool Piece::canMoveUnlimited() const {
        constexpr const IntType unlimMask = ENUM_TO_INT(Piece::Type::Bishop) & ENUM_TO_INT(Piece::Type::Rook) & ENUM_TO_INT(Piece::Type::Queen);
        return (_val & typeMask & unlimMask) != 0;
    }
}
