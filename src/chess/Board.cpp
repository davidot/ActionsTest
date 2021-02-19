#include "Board.h"
#include "../util/StringUtil.h"
#include <string>

namespace Chess {

    Board::Board(uint8_t size) : m_size(size), m_pieces(m_size * m_size, Piece::none()) {
    }

    Board Board::emptyBoard(uint8_t size) {
        return Board(size);
    }

    bool Board::hasValidPosition() const {
        return false;
    }

    int colorIndex(Color c) {
        return c == Color::Black;
    }

    uint32_t Board::countPieces(Color c) const {
        return m_numPieces[colorIndex(c)];
    }

    std::optional<Piece> Board::pieceAt(uint16_t index) {
        if (index >= m_size * m_size) {
            return std::nullopt;
        }

        if (auto& val = m_pieces[index]; Piece::isPiece(val)) {
            return Piece::fromInt(m_pieces[index]);
        } else {
            return std::nullopt;
        }
    }

    void Board::setPiece(uint16_t index, std::optional<Piece> piece) {
        if (index >= m_size * m_size) {
            return;
        }
        if (Piece::isPiece(m_pieces[index])) {
            m_numPieces[colorIndex(Piece::colorFromInt(m_pieces[index]))]--;
        }
        if (piece.has_value()) {
            m_numPieces[colorIndex(piece->color())]++;
            m_pieces[index] = piece->toInt();
        } else {
            m_pieces[index] = Piece::none();
        }
    }

    Board Board::standardBoard() {
        return std::move(Board::fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").extract());
    }

    std::optional<std::string> Board::parseFENBoard(std::string_view view) {
        auto next = view.begin();

        uint32_t index = 0;
        uint32_t row = 1;

        while (next != view.end()) {
            if (index == m_size * row) {
                if (*next != '/') {
                    return "Must have '/' as row separators";
                }
                next++;
            }
            if (std::isalpha(*next)) {
                auto piece = Piece::fromFEN(*next);
                if (!piece) {
                    return std::string("Unknown piece type'") + std::to_string(*next) + std::string("'");
                }
                setPiece(index, *piece);
                index++;
            }

            next++;
        }

        // weirdly nullopt means no error here...
        return std::nullopt;
    }


    ExpectedBoard Board::fromFEN(std::string_view str) {
        Board b(8);

        //rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

        auto parts = util::split(str, " ");
        if (parts.size() < 6) {
            return "Not enough pieces in FEN";
        }

        auto error = b.parseFENBoard(parts[0]);
        if (error) {
            return *error;
        }

        auto& turnString = parts[1];

        if (turnString == "w") {
            b.m_next_turn = Color::White;
        } else if (turnString == "b") {
            b.m_next_turn = Color::Black;
        } else {
            return std::string("Invalid turn value: ") + std::string(turnString);
        }

        return b;
    }

    Color Board::colorToMove() const {
        return m_next_turn;
    }

    uint8_t Board::size() const {
        return m_size;
    }

}
