#include "Board.h"
#include "../util/StringUtil.h"
#include <array>
#include <charconv>
#include <iostream>
#include <string>
#include <sstream>
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

    std::optional<Piece> Board::pieceAt(uint16_t index) const {
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
        uint16_t totalSize = m_size * m_size;

        while (next != view.end() && index <= totalSize) {
            if (index == m_size * row) {
                if (*next != '/') {
                    return "Must have '/' as row separators";
                }
                if (index == totalSize) {
                    return "Must not have trailing '/'";
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
                if (val == 0) {
                    return "Skipping 0 is not allowed";
                }
                if (val > m_size || val > (m_size - index % m_size)) {
                    return "Skipping more than a full row or the current row _" + std::to_string(val) + "_";
                }
                index += val;

                lastWasNum = true;
            }

            ++next;
        }


        if (index > totalSize) {
            return "Board is too long already data for _" + std::to_string(index) + "_ squares";
        } else if (index < totalSize) {
            return "Not enough data to fill board only reached " + std::to_string(index);
        }



        // weirdly nullopt means no error here...
        return std::nullopt;
    }

    char turnColor(Color color) {
        switch (color) {
            case Color::White:
                return 'w';
            case Color::Black:
                return 'b';
        }
        return '?';
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
        if (sv.size() > 1 && sv[0] == '0') {
            // NO LEADING ZEROS!
            return std::nullopt;
        }
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
        if (parts.size() != 6) {
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

        if (!b.setAvailableCastles(parts[2])) {
            return std::string("Invalid possible castling moves value: ") + std::string(parts[2]);
        }

        if (parts[3] != "-") {
            std::optional<uint16_t> enPassantPawn = b.SANToIndex(parts[3]);
            if (!enPassantPawn.has_value()) {
                return std::string("Invalid en passant value: ") + std::string(parts[3]);
            }
            // TODO: check whether this is actually a valid enPassant value (i.e. there is a pawn)
            b.m_enPassant = enPassantPawn;
        }

        std::optional<uint32_t> halfMovesSinceCapture = strictParseUInt(parts[4]);
        if (!halfMovesSinceCapture.has_value()) {
            return std::string("Invalid half moves since capture: ") + std::string(parts[4]);
        }
        b.m_halfMovesSinceCaptureOrPawn = halfMovesSinceCapture.value();

        std::optional<uint32_t> totalFullMoves = strictParseUInt(parts[5]);
        if (!totalFullMoves.has_value()) {
            return std::string("Invalid full moves made: ") + std::string(parts[5]);
        }
        b.m_fullMoveNum = totalFullMoves.value();

        return b;
    }

    Color Board::colorToMove() const {
        return m_next_turn;
    }

    uint8_t Board::size() const {
        return m_size;
    }

    uint16_t Board::columnRowToIndex(uint8_t column, uint8_t row) const {
        return column + uint16_t(m_size) * (uint16_t(m_size) - 1 - row);
    }

    std::pair<uint8_t, uint8_t> Board::indexToColumnRow(uint16_t index) const {
        return std::make_pair(index % m_size, (m_size - 1) - index / m_size);
    }

    std::optional<Piece> Board::pieceAt(uint8_t column, uint8_t row) const {
        return pieceAt(columnRowToIndex(column, row));
    }

    void Board::setPiece(uint8_t column, uint8_t row, std::optional<Piece> piece) {
        setPiece(columnRowToIndex(column, row), piece);
    }

    std::optional<Piece> Board::pieceAt(std::string_view vw) const {
        auto index = SANToIndex(vw);
        if (index) {
            return pieceAt(index.value());
        } else {
            return std::nullopt;
        }
    }

    void Board::setPiece(std::string_view vw, std::optional<Piece> piece) {
        auto index = SANToIndex(vw);
        if (index) {
            setPiece(index.value(), piece);
        }
    }

    std::optional<uint16_t> Board::SANToIndex(std::string_view vw) const {
        if (vw.size() != 2) {
            return std::nullopt;
        }

        if (!std::islower(vw[0]) || !std::isdigit(vw[1])) {
            return std::nullopt;
        }

        if (vw[0] > 'h' || vw[1] > '8' || vw[1] == '0') {
            return std::nullopt;
        }

        auto col = vw[0] - 'a';
        auto row = vw[1] - '1';

        return columnRowToIndex(col, row);
    }

    std::string Board::indexToSAN(uint16_t index) const {
        auto [col, row] = indexToColumnRow(index);
        std::string str;
        str.push_back('a' + col);
        str.push_back('1' + row);
        return str;
    }

    static std::array<std::pair<char, CastlingRight>, 4> castleMapping = {
            std::make_pair(Piece(Piece::Type::King, Color::White).toFEN(), CastlingRight::WHITE_QUEEN_SIDE),
            std::make_pair(Piece(Piece::Type::Queen, Color::White).toFEN(), CastlingRight::WHITE_KING_SIDE),
            std::make_pair(Piece(Piece::Type::King, Color::Black).toFEN(), CastlingRight::BLACK_QUEEN_SIDE),
            std::make_pair(Piece(Piece::Type::Queen, Color::Black).toFEN(), CastlingRight::BLACK_KING_SIDE),
    };

    bool Board::setAvailableCastles(std::string_view vw) {
        if (vw.empty() || vw.size() > 4) {
            return false;
        }
        if (vw == "-") {
            return true;
        }

        auto front = castleMapping.begin();

        for (auto& c : vw) {
            if (auto pos = std::find_if(front, castleMapping.end(), [&](auto pair) {
                    return pair.first == c;
                }); pos == castleMapping.end()) {
                return false;
            } else {
                CastlingRight right = pos->second;
                if ((m_castlingRights & right) != CastlingRight::NO_CASTLING) {
                    // already have that right (Actually not possible anymore)
                    return false;
                }
                m_castlingRights |= right;
                front = std::next(pos);
            }
        }
        return true;
    }

    std::string castlingOutput(CastlingRight right) {
        std::stringstream stream;
        bool any = false;
        for (auto& [fen, fenRight] : castleMapping) {
            if ((right & fenRight) != CastlingRight::NO_CASTLING) {
                stream << fen;
                any = true;
            }
        }
        if (!any) {
            return "-";
        }
        return stream.str();
    }

    std::string Board::toFEN() const {
        std::stringstream val;

        uint8_t emptyAcc = 0;

        auto writeEmpty = [&] {
            if (emptyAcc > 0) {
                val << std::to_string(emptyAcc);
                emptyAcc = 0;
            }
        };

        for (uint8_t row = m_size - 1; row < m_size; row--) {
            for (uint8_t column = 0; column < m_size; column++) {
                auto nextPiece = pieceAt(column, row);
                if (nextPiece) {
                    writeEmpty();
                    val << nextPiece->toFEN();
                } else {
                    emptyAcc++;
                }
            }
            writeEmpty();
            if (row != 0) {
                val << '/';
            }
        }

        val << ' ' << turnColor(m_next_turn)
            << ' ' << castlingOutput(m_castlingRights)
            << ' ' << (m_enPassant.has_value() ? indexToSAN(m_enPassant.value()) : "-")
            << ' ' << m_halfMovesSinceCaptureOrPawn
            << ' ' << m_fullMoveNum;

        return val.str();
    }

#define INT(x) static_cast<uint8_t>(x)
#define TOCASTLE(x) static_cast<CastlingRight>(x)

    CastlingRight& operator|=(CastlingRight& lhs, const CastlingRight& rhs) {
        lhs = TOCASTLE(INT(lhs) | INT(rhs));
        return lhs;
    }

    CastlingRight operator&(const CastlingRight& lhs, const CastlingRight& rhs) {
        return TOCASTLE(INT(lhs) & INT(rhs));
    }


#undef INT
#undef TOCASTLE

}
