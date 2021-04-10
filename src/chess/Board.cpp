#include "Board.h"
#include "../util/Assertions.h"
#include "../util/StringUtil.h"
#include <array>
#include <charconv>
#include <sstream>
#include <string>
#include <utility>
#include <algorithm>

#ifdef OUTPUT_FEN
#include <iostream>
#endif

namespace Chess {

#define INT(x) static_cast<uint8_t>(x)
#define TO_CASTLE(x) static_cast<CastlingRight>(x)

    CastlingRight operator|(const CastlingRight& lhs, const CastlingRight& rhs) {
        return TO_CASTLE(INT(lhs) | INT(rhs));
    }

    CastlingRight& operator|=(CastlingRight& lhs, const CastlingRight& rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    CastlingRight& operator&=(CastlingRight& lhs, const CastlingRight& rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    CastlingRight operator&(const CastlingRight& lhs, const CastlingRight& rhs) {
        return TO_CASTLE(INT(lhs) & INT(rhs));
    }

    // may give invalid castling rights!! so is not defined in .h
    CastlingRight operator~(const CastlingRight& cr) {
        return TO_CASTLE(~INT(cr));
    }

#undef INT
#undef TO_CASTLE


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
            m_pieces[index] = Piece::noneValue();
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

        board.m_castlingRights = CastlingRight::WhiteCastling | CastlingRight::BlackCastling;

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
#ifdef OUTPUT_FEN
        std::cout << str << '\n';
#endif
        Board b{};

        //rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

        std::vector<std::string_view> parts = util::split(str, " ");
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
            std::optional<BoardIndex> enPassantPawn = Board::SANToIndex(parts[3]);
            if (!enPassantPawn.has_value()) {
                return std::string("Invalid en passant value: ") + std::string(parts[3]);
            }
            auto [col, row] = Board::indexToColumnRow(*enPassantPawn);
            Color lastMoveColor = opposite(b.m_nextTurnColor);
            if ((lastMoveColor == Color::White && row != 2) || (lastMoveColor == Color::Black && row != size - 1 - 2)) {
                return std::string("Cannot have en passant on non 3th or 5th row: " + std::string(parts[3]));
            }
            if (b.pieceAt(col, row) != std::nullopt) {
                return "En passant square cannot have a piece at square";
            }
            BoardIndex pawnRow = row + (lastMoveColor == Color::White ? 1 : -1);
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
        if (!totalFullMoves.has_value() || totalFullMoves == 0u) {
            return std::string("Invalid full moves made: ") + std::string(parts[5]);
        }
        b.m_halfMovesMade = (totalFullMoves.value() - 1) * 2 + (b.m_nextTurnColor == Color::Black);

        return b;
    }

    Color Board::colorToMove() const {
        return m_nextTurnColor;
    }

    BoardIndex Board::columnRowToIndex(BoardIndex column, BoardIndex row) {
        return column + uint16_t(size) * (uint16_t(size) - 1 - row);
    }

    std::pair<BoardIndex, BoardIndex> Board::indexToColumnRow(BoardIndex index) {
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

    std::optional<BoardIndex> Board::SANToIndex(std::string_view vw) {
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

    std::optional<std::pair<BoardIndex, BoardIndex>> Board::SANToColRow(std::string_view vw) {
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

    std::string Board::indexToSAN(BoardIndex index) {
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
        BoardIndex col;
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

        return std::all_of(castleChecks.cbegin(), castleChecks.cend(), [&](const auto& check) {
          if ((m_castlingRights & check.right) != CastlingRight::NoCastling) {
              if (pieceAt(check.col, homeRow(check.piece.color())) != check.piece) {
                  return false;
              }
          }
          return true;
        });
    }

    std::ostream& operator<<(std::ostream& stream, const CastlingRight& right) {
        bool any = false;
        for (auto& [fen, fenRight] : castleMapping) {
            if ((right & fenRight) != CastlingRight::NoCastling) {
                stream << fen;
                any = true;
            }
        }
        if (!any) {
            stream << '-';
        }
        return stream;
    }

    std::optional<std::pair<BoardIndex, BoardIndex>> Board::enPassantColRow() const {
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
            << ' ' << m_castlingRights
            << ' ' << (m_enPassant.has_value() ? indexToSAN(m_enPassant.value()) : "-")
            << ' ' << m_halfMovesSinceCaptureOrPawn
            << ' ' << fullMoves();

        return val.str();
    }

    void Board::makeNullMove() {
        m_nextTurnColor = opposite(m_nextTurnColor);
        ++m_halfMovesSinceCaptureOrPawn;
        ++m_halfMovesMade;
    }

    void Board::undoNullMove() {
        m_nextTurnColor = opposite(m_nextTurnColor);
        ASSERT(m_halfMovesMade > 0);
        ASSERT(m_halfMovesSinceCaptureOrPawn > 0);
        --m_halfMovesSinceCaptureOrPawn;
        --m_halfMovesMade;
    }

    CastlingRight Board::castlingRights() const {
        return m_castlingRights;
    }

    std::pair<BoardIndex, BoardIndex> Board::kingSquare(Color color) const {
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

    uint32_t Board::fullMoves() const {
        return m_halfMovesMade / 2 + 1;
    }
    uint32_t Board::halfMovesSinceIrreversible() const {
        return m_halfMovesSinceCaptureOrPawn;
    }

    bool Board::operator==(const Board &rhs) const {
        return m_halfMovesMade == rhs.m_halfMovesMade
            && m_halfMovesSinceCaptureOrPawn == rhs.m_halfMovesSinceCaptureOrPawn
            && m_castlingRights == rhs.m_castlingRights
            && m_enPassant == rhs.m_enPassant
            && m_nextTurnColor == rhs.m_nextTurnColor
            && m_pieces == rhs.m_pieces;
    }

    bool Board::makeMove(Move m) {
        ASSERT(m.fromPosition != m.toPosition);
        ASSERT(pieceAt(m.fromPosition).has_value()
               && pieceAt(m.fromPosition)->color() == m_nextTurnColor);

        Piece p = pieceAt(m.fromPosition).value();
        MoveData data{};

        data.previousEnPassant = m_enPassant;
        data.previousCastlingRights = m_castlingRights;
        data.performedMove = m;


        auto [colFrom, rowFrom] = indexToColumnRow(m.fromPosition);
        auto [colTo, rowTo] = indexToColumnRow(m.toPosition);


        if (m.flag == Move::Flag::Castling) {
            ASSERT(p.type() == Piece::Type::King);
            ASSERT(rowFrom == rowTo);
            ASSERT(pieceAt(m.toPosition) == Piece(Piece::Type::Rook, m_nextTurnColor));
            // again we assume this is a legal move so just perform it
            if (colFrom < colTo) {
                // king side
                ASSERT(colTo == Board::kingSideRookCol);
                setPiece(colFrom, rowFrom, std::nullopt);
                setPiece(colFrom + 2, rowFrom, Piece{Piece::Type::King, m_nextTurnColor});
                setPiece(colTo, rowFrom, std::nullopt);
                setPiece(colFrom + 1, rowFrom, Piece{Piece::Type::Rook, m_nextTurnColor});
            } else {
                ASSERT(colTo == Board::queenSideRookCol);
                setPiece(colFrom, rowFrom, std::nullopt);
                setPiece(colFrom - 2, rowFrom, Piece{Piece::Type::King, m_nextTurnColor});
                setPiece(colTo, rowFrom, std::nullopt);
                setPiece(colFrom - 1, rowFrom, Piece{Piece::Type::Rook, m_nextTurnColor});
            }
        } else {
            data.capturedPiece = pieceAt(m.toPosition);
            ASSERT(!data.capturedPiece.has_value()
                   || data.capturedPiece->color() != m_nextTurnColor);

            setPiece(m.toPosition, p);
            setPiece(m.fromPosition, std::nullopt);
        }


        if (m.isPromotion()) {
            setPiece(m.toPosition, Piece{m.promotedType(), m_nextTurnColor});
        }

        if (m.flag == Move::Flag::EnPassant) {
            ASSERT(pieceAt(m.toPosition)->type() == Piece::Type::Pawn);

            ASSERT(pieceAt(colTo, rowFrom).has_value()
                   && pieceAt(colTo, rowFrom)->type() == Piece::Type::Pawn
                   && pieceAt(colTo, rowFrom)->color() == opposite(m_nextTurnColor));

            setPiece(columnRowToIndex(colTo, rowFrom), std::nullopt);
        }

        if (m.flag == Move::Flag::DoublePushPawn) {
            m_enPassant = columnRowToIndex(colFrom, rowFrom + pawnDirection(m_nextTurnColor));
        } else {
            m_enPassant = std::nullopt;
        }

        auto removeCastlingRights = [&](BoardIndex col, BoardIndex row) {
            CastlingRight base;
            if (row == homeRow(Color::White)) {
                base = CastlingRight::WhiteCastling;
            } else if (row == homeRow(Color::Black)) {
                base = CastlingRight::BlackCastling;
            } else {
                return;
            }
            switch (col) {
                case kingCol:
                    m_castlingRights &= ~base;
                    break;
                case kingSideRookCol:
                    m_castlingRights &= ~(base & CastlingRight::KingSideCastling);
                    break;
                case queenSideRookCol:
                    m_castlingRights &= ~(base & CastlingRight::QueenSideCastling);
                    break;
                default:
                    break;
            }
        };

        removeCastlingRights(colFrom, rowFrom);
        removeCastlingRights(colTo, rowTo);

        m_nextTurnColor = opposite(m_nextTurnColor);
        m_history.push_back(data);
        ++m_halfMovesMade;
        ++m_halfMovesSinceCaptureOrPawn;

        return true;
    }

    bool Board::undoMove() {
        if (m_history.empty()) {
            return false;
        }

        m_nextTurnColor = opposite(m_nextTurnColor);

        MoveData data = m_history.back();
        m_history.pop_back();

        Move& m = data.performedMove;

        if (m.flag == Move::Flag::Castling) {
            auto [colFrom, rowFrom] = indexToColumnRow(m.fromPosition);
            auto [colTo, rowTo] = indexToColumnRow(m.toPosition);
            if (colFrom < colTo) {
                // king side
                ASSERT(colTo == Board::kingSideRookCol);
                ASSERT(pieceAt(colFrom + 2, rowFrom) == Piece(Piece::Type::King, m_nextTurnColor));
                ASSERT(pieceAt(colFrom + 1, rowFrom) == Piece(Piece::Type::Rook, m_nextTurnColor));

                setPiece(colFrom + 2, rowFrom, std::nullopt);
                setPiece(colFrom + 1, rowFrom, std::nullopt);
                setPiece(colFrom, rowFrom, Piece{Piece::Type::King, m_nextTurnColor});
                setPiece(colTo, rowFrom, Piece{Piece::Type::Rook, m_nextTurnColor});
            } else {
                ASSERT(colTo == Board::queenSideRookCol);
                ASSERT(pieceAt(colFrom - 2, rowFrom) == Piece(Piece::Type::King, m_nextTurnColor));
                ASSERT(pieceAt(colFrom - 1, rowFrom) == Piece(Piece::Type::Rook, m_nextTurnColor));

                setPiece(colFrom - 2, rowFrom, std::nullopt);
                setPiece(colFrom - 1, rowFrom, std::nullopt);
                setPiece(colFrom, rowFrom, Piece{Piece::Type::King, m_nextTurnColor});
                setPiece(colTo, rowFrom, Piece{Piece::Type::Rook, m_nextTurnColor});
            }
        } else {
            ASSERT(pieceAt(m.toPosition).has_value());
            Piece p = pieceAt(m.toPosition).value();
            setPiece(m.fromPosition, p);
            setPiece(m.toPosition, data.capturedPiece);
        }

        if (m.isPromotion()) {
            setPiece(m.fromPosition, Piece{Piece::Type::Pawn, m_nextTurnColor});
        }

        if (m.flag == Move::Flag::EnPassant) {
            auto [colFrom, rowFrom] = indexToColumnRow(m.fromPosition);
            auto [colTo, rowTo] = indexToColumnRow(m.toPosition);

            ASSERT(!pieceAt(colTo, rowFrom).has_value());
            setPiece(colTo, rowFrom, Piece{Piece::Type::Pawn, opposite(m_nextTurnColor)});
        }

        m_enPassant = data.previousEnPassant;
        m_castlingRights = data.previousCastlingRights;

        --m_halfMovesMade;
        // TODO: wrong!
        --m_halfMovesSinceCaptureOrPawn;

        return true;
    }

    Move::Move(BoardIndex fromIndex, BoardIndex toIndex, Flag flags)
        :  toPosition(toIndex),
           fromPosition(fromIndex),
          flag(flags) {
    }

    Move::Move(BoardIndex fromCol, BoardIndex fromRow, BoardOffset offset, Move::Flag flags) : flag(flags) {
        BoardIndex index = Board::columnRowToIndex(fromCol, fromRow);
        toPosition = index + offset;
        fromPosition = index;
    }

    Move::Move(BoardIndex fromCol, BoardIndex fromRow, BoardIndex toCol, BoardIndex toRow, Move::Flag flags) :
        toPosition(Board::columnRowToIndex(toCol, toRow)),
        fromPosition(Board::columnRowToIndex(fromCol, fromRow)),
        flag(flags) {
    }

    Move::Move() : toPosition(0), fromPosition(0), flag(Flag::None) {
    }

    bool Move::isPromotion() const {
        return (static_cast<uint8_t>(flag) & 0x4u) != 0;
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
                break;
        }
        ASSERT_NOT_REACHED();
    }

    std::pair<BoardIndex, BoardIndex> Move::colRowFromPosition() const {
        return Board::indexToColumnRow(fromPosition);
    }

    std::pair<BoardIndex, BoardIndex> Move::colRowToPosition() const {
        return Board::indexToColumnRow(toPosition);
    }

    std::string Move::toSANSquares() const {
        return Board::indexToSAN(fromPosition) + Board::indexToSAN(toPosition);
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
