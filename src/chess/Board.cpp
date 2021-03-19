#include "Board.h"
#include "../util/Assertions.h"
#include "../util/StringUtil.h"
#include <array>
#include <charconv>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace Chess {

    Board Board::emptyBoard() {
        return Board();
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

    std::optional<Piece> Board::pieceAt(BoardIndex index) const {
        if (index >= m_size * m_size) {
            return std::nullopt;
        }

        if (auto& val = m_pieces[index]; Piece::isPiece(val)) {
            return Piece::fromInt(m_pieces[index]);
        } else {
            return std::nullopt;
        }
    }

    void Board::setPiece(BoardIndex index, std::optional<Piece> piece) {
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
        return Board::fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").extract();
    }

    std::optional<std::string> Board::parseFENBoard(std::string_view view) {
        auto next = view.begin();

        uint32_t index = 0;
        uint32_t row = 1;
        bool lastWasNum = false;
        uint16_t totalSize = m_size * m_size;

        while (next != view.end() && index <= totalSize) {
            if (index == m_size * row) {
                if (index == totalSize) {
                    if (*next == '/') {
                        return "Must not have trailing '/'";
                    }
                    return "Board is too long already data for _" + std::to_string(index) + "_ squares";
                }
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
                uint32_t val = *next - '0';
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


        if (index < totalSize) {
            return "Not enough data to fill board only reached " + std::to_string(index);
        }



        // weirdly nullopt means no error here...
        return std::nullopt;
    }

    char turnColor(Color color) {
        if (color == Color::White) {
            return 'w';
        }
        return 'b';
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
        Board b{};

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
        b.m_nextTurnColor = nextTurn.value();

        if (!b.setAvailableCastles(parts[2])) {
            return std::string("Invalid possible castling moves value: ") + std::string(parts[2]);
        }

        if (parts[3] != "-") {
            std::optional<BoardIndex> enPassantPawn = b.SANToIndex(parts[3]);
            if (!enPassantPawn.has_value()) {
                return std::string("Invalid en passant value: ") + std::string(parts[3]);
            }
            auto [col, row] = b.indexToColumnRow(*enPassantPawn);
            if (row != 2 && row != m_size - 3) {
                return std::string("Cannot have en passant on non 3th or 5th row: " + std::string(parts[3]));
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
        return m_nextTurnColor;
    }

    uint8_t Board::size() const {
        return m_size;
    }

    Board::BoardIndex Board::columnRowToIndex(BoardIndex column, BoardIndex row) {
        return column + uint16_t(m_size) * (uint16_t(m_size) - 1 - row);
    }

    std::pair<Board::BoardIndex, Board::BoardIndex> Board::indexToColumnRow(BoardIndex index) {
        return std::make_pair(index % m_size, (m_size - 1) - index / m_size);
    }

    std::optional<Piece> Board::pieceAt(BoardIndex column, BoardIndex row) const {
        if (column >= m_size || row >= m_size) {
            return std::nullopt;
        }
        return pieceAt(columnRowToIndex(column, row));
    }

    std::optional<Piece> Board::pieceAt(std::pair<BoardIndex, BoardIndex> coords) const {
        return pieceAt(coords.first, coords.second);
    }

    void Board::setPiece(BoardIndex column, BoardIndex row, std::optional<Piece> piece) {
        if (column >= m_size || row >= m_size) {
            return;
        }
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

    std::optional<Board::BoardIndex> Board::SANToIndex(std::string_view vw) {
        if (vw.size() != 2) {
            return std::nullopt;
        }

        if (!std::islower(vw[0]) || !std::isdigit(vw[1])) {
            return std::nullopt;
        }

        if (vw[0] > 'h' || vw[1] == '9' || vw[1] == '0') {
            return std::nullopt;
        }

        auto col = vw[0] - 'a';
        auto row = vw[1] - '1';

        return columnRowToIndex(col, row);
    }

    std::optional<std::pair<Board::BoardIndex, Board::BoardIndex>> Board::SANToColRow(std::string_view vw) {
        auto pos = SANToIndex(vw);
        if (pos) {
            return indexToColumnRow(pos.value());
        }
        return std::nullopt;
    }

    std::string Board::columnRowToSAN(BoardIndex col, BoardIndex row) {
        std::string str;
        str.push_back('a' + col);
        str.push_back('1' + row);
        return str;
    }

    std::string Board::indexToSAN(Board::BoardIndex index) {
        auto [col, row] = indexToColumnRow(index);
        return columnRowToSAN(col, row);
    }

    std::array<std::pair<char, CastlingRight>, 4> castleMapping = {
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

        //reset before hand
        m_castlingRights = CastlingRight::NO_CASTLING;

        auto front = castleMapping.begin();

        for (auto& c : vw) {
            if (auto pos = std::find_if(front, castleMapping.end(), [&](auto pair) {
                    return pair.first == c;
                }); pos == castleMapping.end()) {
                return false;
            } else {
                m_castlingRights |= pos->second;
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

    std::optional<std::pair<Board::BoardIndex, Board::BoardIndex>> Board::enPassantColRow() const {
        if (!m_enPassant.has_value()) {
            return std::nullopt;
        }
        return indexToColumnRow(*m_enPassant);
    }

    std::string Board::toFEN() const {
        std::stringstream val;

        BoardIndex emptyAcc = 0;

        auto writeEmpty = [&] {
            if (emptyAcc > 0) {
                val << std::to_string(emptyAcc);
                emptyAcc = 0;
            }
        };

        for (BoardIndex row = m_size - 1; row < m_size; row--) {
            for (BoardIndex column = 0; column < m_size; column++) {
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

        val << ' ' << turnColor(m_nextTurnColor)
            << ' ' << castlingOutput(m_castlingRights)
            << ' ' << (m_enPassant.has_value() ? indexToSAN(m_enPassant.value()) : "-")
            << ' ' << m_halfMovesSinceCaptureOrPawn
            << ' ' << m_fullMoveNum;

        return val.str();
    }

    void Board::makeNullMove() {
        m_nextTurnColor = opposite(m_nextTurnColor);
    }

    void Board::undoNullMove() {
        m_nextTurnColor = opposite(m_nextTurnColor);
    }
    CastlingRight Board::castlingRights() {
        return m_castlingRights;
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

    Move::Move(Board::BoardIndex fromPosition, Board::BoardIndex toPosition, Flag flags)
        :  toPosition(toPosition),
           fromPosition(fromPosition),
          flag(flags) {
    }

    Move::Move(Board::BoardIndex fromCol, Board::BoardIndex fromRow, BoardOffset offset, Move::Flag flags) : flag(flags) {
        Board::BoardIndex index = Board::columnRowToIndex(fromCol, fromRow);
        toPosition = index + offset;
        fromPosition = index;
    }

    Move::Move(Board::BoardIndex fromCol, Board::BoardIndex fromRow, Board::BoardIndex toCol, Board::BoardIndex toRow, Move::Flag flags) :
        toPosition(Board::columnRowToIndex(toCol, toRow)),
        fromPosition(Board::columnRowToIndex(fromCol, fromRow)),
                                                                                                                                            flag(flags) {
    }
    Move::Move() : toPosition(0), fromPosition(0), flag(Flag::None) {
    }

    bool Move::isPromotion() const {
        return (static_cast<uint8_t>(flag) & 0x4) != 0;
    }

    Piece::Type Move::promotedType() const {
        switch (flag) {
            case Flag::PromotionToKnight:
                return Piece::Type::Knight;
            case Flag::PromotionToBishop:
                return Piece::Type::Bishop;
            case Flag::PromotionToRook:
                return Piece::Type::Rook;
            case Flag::PromotionToQueen:
                return Piece::Type::Queen;
            default:
                VERIFY_NOT_REACHED();
        }
    }

    std::pair<Board::BoardIndex, Board::BoardIndex> Move::colRowFromPosition() const {
        return Board::indexToColumnRow(fromPosition);
    }

    std::pair<Board::BoardIndex, Board::BoardIndex> Move::colRowToPosition() const {
        return Board::indexToColumnRow(toPosition);
    }


    const std::string &ExpectedBoard::error() const {
        ASSERT(m_value.index() == 1);
        // in case it is a string we need to use the indices
        return std::get<1>(m_value);
    }

    ExpectedBoard::T &&ExpectedBoard::extract() {
        ASSERT(m_value.index() == 0);
        // in case it is a string we need to use the indices
        return std::move(std::get<0>(m_value));
    }

    const ExpectedBoard::T &ExpectedBoard::value() {
        ASSERT(m_value.index() == 0);
        // in case it is a string we need to use the indices
        return std::get<0>(m_value);
    }
}
