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
        auto fen = toFEN();
        auto copied = Board::fromFEN(fen);
        if (!copied) {
            return false;
        }
        // TODO: check at least one king on both sides
        // TODO: check king in non turn color is NOT in check (can take king)
        return countPieces(Color::White) > 0 && countPieces(Color::Black);
    }

    int colorIndex(Color c) {
        return c == Color::Black;
    }

    uint32_t Board::countPieces(Color c) const {
        return m_numPieces[colorIndex(c)];
    }

    std::optional<Piece> Board::pieceAt(BoardIndex index) const {
        if (index >= size * size) {
            return std::nullopt;
        }

        if (auto& val = m_pieces[index]; Piece::isPiece(val)) {
            return Piece::fromInt(m_pieces[index]);
        } else {
            return std::nullopt;
        }
    }

    void Board::setPiece(BoardIndex index, std::optional<Piece> piece) {
        if (index >= size * size) {
            return;
        }
        if (auto p = pieceAt(index); p) {
            m_numPieces[colorIndex(p->color())]--;
#ifdef STORE_KING_POS
            if (p->type() == Piece::Type::King) {
                m_kingPos[colorIndex(p->color())] = -1;
            }
#endif
        }
        if (piece.has_value()) {
            m_numPieces[colorIndex(piece->color())]++;
            m_pieces[index] = piece->toInt();
#ifdef STORE_KING_POS
            if (piece->type() == Piece::Type::King) {
                m_kingPos[colorIndex(piece->color())] = index;
            }
#endif
        } else {
            m_pieces[index] = Piece::none();
        }
    }

    Board Board::standardBoard() {
        Board board{};

        auto bPawnRow = pawnHomeRow(Color::Black);
        auto wPawnRow = pawnHomeRow(Color::White);

        for (BoardIndex col = 0; col < size; col++) {
            board.setPiece(col, bPawnRow, Piece{Piece::Type::Pawn, Color::Black});
            board.setPiece(col, wPawnRow, Piece{Piece::Type::Pawn, Color::White});
        }

        auto bHome = homeRow(Color::Black);
        auto wHome = homeRow(Color::White);

        BoardIndex col = 0;
        for (Piece::Type tp : {Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop}) {
            board.setPiece(col, bHome, Piece{tp, Color::Black});
            board.setPiece(size - 1 - col, bHome, Piece{tp, Color::Black});

            board.setPiece(col, wHome, Piece{tp, Color::White});
            board.setPiece(size - 1 - col, wHome, Piece{tp, Color::White});
            col++;
        }

        board.setPiece(kingCol, bHome, Piece{Piece::Type::King, Color::Black});
        board.setPiece(kingCol, wHome, Piece{Piece::Type::King, Color::White});

        board.setPiece(kingCol - 1, bHome, Piece{Piece::Type::Queen, Color::Black});
        board.setPiece(kingCol - 1, wHome, Piece{Piece::Type::Queen, Color::White});

        board.m_castlingRights |= CastlingRight::WhiteCastling;
        board.m_castlingRights |= CastlingRight::BlackCastling;

        return board;
    }

    std::optional<std::string> Board::parseFENBoard(std::string_view view) {
        auto next = view.begin();

        uint32_t index = 0;
        uint32_t row = 1;
        bool lastWasNum = false;
        uint16_t totalSize = size * size;

        while (next != view.end() && index <= totalSize) {
            if (index == size * row) {
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
                if (val > size || val > (size - index % size)) {
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
            return error.value();
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
            auto lastMoveColor = opposite(b.m_nextTurnColor);
            if ((lastMoveColor == Color::White && row != 2) || (lastMoveColor == Color::Black && row != size - 1 - 2)) {
                return std::string("Cannot have en passant on non 3th or 5th row: " + std::string(parts[3]));
            }
            if (b.pieceAt(col, row) != std::nullopt) {
                return "En passant square cannot have a piece at square";
            }
            auto pawnRow = row + (lastMoveColor == Color::White ? 1 : -1);
            if (b.pieceAt(col, pawnRow) != Piece{Piece::Type::Pawn, lastMoveColor}) {
                return "En passant square must be just behind previously moved pawn";
            }
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

    Board::BoardIndex Board::columnRowToIndex(BoardIndex column, BoardIndex row) {
        return column + uint16_t(size) * (uint16_t(size) - 1 - row);
    }

    std::pair<Board::BoardIndex, Board::BoardIndex> Board::indexToColumnRow(BoardIndex index) {
        return std::make_pair(index % size, (size - 1) - index / size);
    }

    std::optional<Piece> Board::pieceAt(BoardIndex column, BoardIndex row) const {
        if (column >= size || row >= size) {
            return std::nullopt;
        }
        return pieceAt(columnRowToIndex(column, row));
    }

    std::optional<Piece> Board::pieceAt(std::pair<BoardIndex, BoardIndex> coords) const {
        return pieceAt(coords.first, coords.second);
    }

    void Board::setPiece(BoardIndex column, BoardIndex row, std::optional<Piece> piece) {
        if (column >= size || row >= size) {
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

    const std::array<std::pair<char, CastlingRight>, 4> castleMapping = {
            std::make_pair(Piece(Piece::Type::King, Color::White).toFEN(), CastlingRight::WhiteKingSide),
            std::make_pair(Piece(Piece::Type::Queen, Color::White).toFEN(), CastlingRight::WhiteQueenSide),
            std::make_pair(Piece(Piece::Type::King, Color::Black).toFEN(), CastlingRight::BlackKingSide),
            std::make_pair(Piece(Piece::Type::Queen, Color::Black).toFEN(), CastlingRight::BlackQueenSide),
    };

    struct CstlChk {
        CastlingRight right;
        Board::BoardIndex col;
        Piece piece;
    };

    const std::array<CstlChk, 6> castleChecks = {
            CstlChk{CastlingRight::WhiteCastling, Board::kingCol, Piece{Piece::Type::King, Color::White}},
            CstlChk{CastlingRight::WhiteQueenSide, Board::queenSideRookCol, Piece{Piece::Type::Rook, Color::White}},
            CstlChk{CastlingRight::WhiteKingSide, Board::kingSideRookCol, Piece{Piece::Type::Rook, Color::White}},
            CstlChk{CastlingRight::BlackCastling, Board::kingCol, Piece{Piece::Type::King, Color::Black}},
            CstlChk{CastlingRight::BlackQueenSide, Board::queenSideRookCol, Piece{Piece::Type::Rook, Color::Black}},
            CstlChk{CastlingRight::BlackKingSide, Board::kingSideRookCol, Piece{Piece::Type::Rook, Color::Black}},
    };

    bool Board::setAvailableCastles(std::string_view vw) {
        if (vw.empty() || vw.size() > 4) {
            return false;
        }
        if (vw == "-") {
            return true;
        }
        //reset before hand
        m_castlingRights = CastlingRight::NoCastling;

        auto front = castleMapping.begin();

        for (const auto& c : vw) {
            if (auto pos = std::find_if(front, castleMapping.end(), [&](auto pair) {
                    return pair.first == c;
                }); pos == castleMapping.end()) {
                return false;
            } else {
                m_castlingRights |= pos->second;
                front = std::next(pos);
            }
        }

        for (const auto& check : castleChecks) {
            if ((m_castlingRights & check.right) != CastlingRight::NoCastling) {
                if (pieceAt(check.col, homeRow(check.piece.color())) != check.piece) {
                    return false;
                }
            }
        }
        return true;
    }

    std::string castlingOutput(CastlingRight right) {
        std::stringstream stream;
        bool any = false;
        for (auto& [fen, fenRight] : castleMapping) {
            if ((right & fenRight) != CastlingRight::NoCastling) {
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

        for (BoardIndex row = size - 1; row < size; row--) {
            for (BoardIndex column = 0; column < size; column++) {
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

    CastlingRight Board::castlingRights() const {
        return m_castlingRights;
    }

    std::pair<Board::BoardIndex, Board::BoardIndex> Board::kingSquare(Color color) const {
#ifdef STORE_KING_POS
        return indexToColumnRow(m_kingPos[colorIndex(color)]);
#else
        Piece king {Piece::Type::King, color};
        for (uint8_t col = 0; col < size; col++) {
            for (uint8_t row = 0; row < size; row++) {
                if (pieceAt(col, row) == king) {
                    // if there are multiple kings we dont care
                    return std::make_pair(col, row);
                }
            }
        }
        return std::make_pair(size + 1, size + 1);
#endif
    }

    bool Board::operator==(const Board &rhs) const {
        return m_fullMoveNum == rhs.m_fullMoveNum
            && m_halfMovesSinceCaptureOrPawn == rhs.m_halfMovesSinceCaptureOrPawn
            && m_castlingRights == rhs.m_castlingRights
            && m_enPassant == rhs.m_enPassant
            && m_nextTurnColor == rhs.m_nextTurnColor
            && m_pieces == rhs.m_pieces;
    }

    Board::BoardIndex Board::homeRow(Color color) {
        return color == Color::White ? 0 : 7;
    }

    Board::BoardOffset Board::pawnDirection(Color color) {
        return color == Color::White ? 1 : -1;
    }

    Board::BoardIndex Board::pawnHomeRow(Color color) {
        return homeRow(color) + pawnDirection(color);
    }

    Board::BoardIndex Board::pawnPromotionRow(Color color) {
        return pawnHomeRow(opposite(color)) + pawnDirection(color);
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

    Move::Move(Board::BoardIndex fromCol, Board::BoardIndex fromRow, Board::BoardOffset offset, Move::Flag flags) : flag(flags) {
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
                ASSERT_NOT_REACHED();
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
