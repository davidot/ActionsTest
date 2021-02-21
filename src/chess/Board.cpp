#include "Board.h"
#include "../util/StringUtil.h"
#include <string>
#include <iostream>
#include <charconv>
#include <utility>

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
        bool lastWasNum = false;

        while (next != view.end()) {
            if (index >= 64) {
                return "Board is too long already data for _" + std::to_string(index) + "_ squares";
            }
            if (index == m_size * row) {
                if (*next != '/') {
                    return "Must have '/' as row separators";
                }
                ++next;
                ++row;
                lastWasNum = false;
                // just in case it was the final char
                continue;
            }
            if (std::isalpha(*next)) {
                auto piece = Piece::fromFEN(*next);
                if (!piece) {
                    return "Unknown piece type'" + std::string(1, *next) + "'";
                }
                setPiece(index, *piece);
                ++index;
                lastWasNum = false;
            } else {
                if (!std::isdigit(*next)) {
                    return "Invalid character '" + std::string(1, *next) + "'";
                }
                if (lastWasNum) {
                    return "Multiple consecutive numbers is not allowed";
                }
                auto val = *next - '0';
                if (val > m_size) {
                    return "Skipping more than a full row _" + std::to_string(val) + "_";
                }
                index += val;

                lastWasNum = true;
            }

            ++next;
        }

        if (index < 64) {
            return "Not enough data to fill board only reached " + std::to_string(index);
        }

        // weirdly nullopt means no error here...
        return std::nullopt;
    }

    std::optional<Color> parseTurnColor(const std::string_view& vw) {
        if (vw.size() != 1) {
            return std::nullopt;
        }

        if (vw[0] == 'w') {
            return Color::White;
        } else if (vw[0] == 'b') {
            return Color::Black;
        }

        return std::nullopt;
    }

    std::optional<uint32_t> strictParseUInt(const std::string_view& sv) {
        uint32_t result;
        if(auto [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
                ec == std::errc()) {
            if (p != sv.data() + sv.size()) {
                return std::nullopt;
            }
            return result;
        }
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

        std::optional<Color> nextTurn = parseTurnColor(parts[1]);
        if (!nextTurn.has_value()) {
            return std::string("Invalid turn value: ") + std::string(parts[1]);
        }
        b.m_next_turn = nextTurn.value();

        auto& castlingString = parts[2];

        auto& enPassantString = parts[3];

        std::optional<uint32_t> halfMovesSinceCapture = strictParseUInt(parts[4]);
        if (!halfMovesSinceCapture.has_value()) {
            return std::string("Invalid half moves since capture: ") + std::string(parts[4]);
        }

        std::optional<uint32_t> totalFullMoves = strictParseUInt(parts[5]);
        if (!totalFullMoves.has_value()) {
            return std::string("Invalid full moves made: ") + std::string(parts[4]);
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
