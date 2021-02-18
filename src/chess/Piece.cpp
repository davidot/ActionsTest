#include "Piece.h"

namespace Chess {

    Color opposite(Color c) {
        switch (c) {
            case Color::White:
                return Color::Black;
            case Color::Black:
                return Color::White;
        }
    }

#define ENUM_TO_INT(val) static_cast<Piece::IntType>(val)
    constexpr Piece::IntType typeMask = 0b1111;
    constexpr Piece::IntType whiteMask = ENUM_TO_INT(Color::White);
    constexpr Piece::IntType blackMask = ENUM_TO_INT(Color::Black);

    constexpr Piece::IntType colorMask = whiteMask | blackMask;

    Piece::Piece(Piece::Type tp, Color c) {
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

    char Piece::toFEN() const {
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
                c = '?';
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
                return T::Pawn;
        }
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
        constexpr const auto pawnMask = ENUM_TO_INT(Piece::Type::Pawn);
        return (m_val & typeMask) == pawnMask;
    }
    bool Piece::canKnightJump() const {
        constexpr const auto knightMask = ENUM_TO_INT(Piece::Type::Knight);
        return (m_val & typeMask) == knightMask;
    }
    bool Piece::canMoveDiagonally() const {
        constexpr const IntType diagMask = ENUM_TO_INT(Piece::Type::Bishop) & ENUM_TO_INT(Piece::Type::Queen) & ENUM_TO_INT(Piece::Type::King);
        return (m_val & typeMask & diagMask) != 0;
    }
    bool Piece::canMoveAxisAligned() const {
        constexpr const IntType axisMask = ENUM_TO_INT(Piece::Type::Rook) & ENUM_TO_INT(Piece::Type::Queen) & ENUM_TO_INT(Piece::Type::King);
        return (m_val & typeMask & axisMask) != 0;
    }
    bool Piece::canMoveUnlimited() const {
        constexpr const IntType unlimMask = ENUM_TO_INT(Piece::Type::Bishop) & ENUM_TO_INT(Piece::Type::Rook) & ENUM_TO_INT(Piece::Type::Queen);
        return (m_val & typeMask & unlimMask) != 0;
    }

    bool Piece::isPiece(Piece::IntType val) {
        return ((val & whiteMask) ^ ((val & blackMask) >> 1)) != 0;
    }

    Piece::IntType Piece::none() {
        return 0;
    }

    Color Piece::colorFromInt(Piece::IntType val) {
        //ASSERT(isPiece(val));
        switch (val & colorMask) {
            case whiteMask:
                return Color::White;
            case blackMask:
                return Color::Black;
        }
        return Color::White;
    }

}
