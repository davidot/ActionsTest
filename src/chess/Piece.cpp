#include "Piece.h"

namespace Chess {
    Piece::Piece(Piece::Type tp, Piece::Color c) : _type(tp), _color(c) {
    }

    uint16_t Piece::toInt() {
        return 0;
    }

    Piece Piece::fromFEN(char c) {
        return Piece(Piece::Rook, Piece::Color::White);
    }

    uint16_t Piece::intFromFEN(char c) {
        return 0;
    }

    Piece Piece::fromInt(uint16_t) {
        return Piece(Piece::Rook, Piece::Color::White);
    }

    bool Piece::operator==(const Piece &rhs) const {
        return _type == rhs._type &&
               _color == rhs._color;
    }

    bool Piece::operator!=(const Piece &rhs) const {
        return !(rhs == *this);
    }

    char Piece::toFEN() {
        return 'a';
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
