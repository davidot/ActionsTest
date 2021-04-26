#include "Board.h"
#include "../util/Assertions.h"
#include <string>
#include <utility>

#ifdef OUTPUT_FEN
#include <iostream>
#endif

namespace Chess {

    CastlingRight operator|(const CastlingRight& lhs, const CastlingRight& rhs) {
        return static_cast<CastlingRight>(
            static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
        );
    }

    CastlingRight operator&(const CastlingRight& lhs, const CastlingRight& rhs) {
        return static_cast<CastlingRight>(
            static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
        );
    }

    CastlingRight& operator&=(CastlingRight& lhs, const CastlingRight& rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    // may give invalid castling rights!! so is not defined in .h
    CastlingRight operator~(const CastlingRight& cr) {
        return static_cast<CastlingRight>(~static_cast<uint8_t>(cr));
    }


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

    Color Board::colorToMove() const {
        return m_nextTurnColor;
    }
    BoardIndex Board::columnRowToIndex(BoardIndex column, BoardIndex row) {
        return column + size * row;
    }

    std::pair<BoardIndex, BoardIndex> Board::indexToColumnRow(BoardIndex index) {
        return std::make_pair(index % size, index / size);
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

    std::optional<std::pair<BoardIndex, BoardIndex>> Board::enPassantColRow() const {
        if (!m_enPassant.has_value()) {
            return std::nullopt;
        }
        return indexToColumnRow(*m_enPassant);
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

    Board::MoveData::MoveData(const Board& board, Move move) :
        performedMove(move),
        previousEnPassant(board.m_enPassant),
        previousCastlingRights(board.m_castlingRights),
        previousSinceCapture(board.m_halfMovesSinceCaptureOrPawn),
        timesRepeated(board.m_repeated) {
    }

    void Board::MoveData::takeValues(Board& board) {
        board.m_enPassant = previousEnPassant;
        board.m_castlingRights = previousCastlingRights;
        board.m_halfMovesSinceCaptureOrPawn = previousSinceCapture;
        board.m_repeated = timesRepeated;
    }

    bool Board::makeMove(Move m) {
        ASSERT(m.fromPosition != m.toPosition);
        ASSERT(pieceAt(m.fromPosition).has_value()
               && pieceAt(m.fromPosition)->color() == m_nextTurnColor);

        Piece p = pieceAt(m.fromPosition).value();
        MoveData data{*this, m};

        ++m_halfMovesMade;
        ++m_halfMovesSinceCaptureOrPawn;

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

        if (p.type() == Piece::Type::Pawn || data.capturedPiece.has_value()) {
            m_halfMovesSinceCaptureOrPawn = 0;
        }

        m_history.push_back(data);

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

        m_repeated = findRepetitions();

        m_nextTurnColor = opposite(m_nextTurnColor);

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

        data.takeValues(*this);

        ASSERT(m_halfMovesMade > 0);
        --m_halfMovesMade;

        return true;
    }

    uint32_t Board::positionRepeated() const {
        return m_repeated;
    }

    uint32_t Board::findRepetitions() const {
        uint32_t maxMoves = std::min<size_t>(m_halfMovesSinceCaptureOrPawn, m_history.size());

        if (maxMoves < 4) {
            return 0;
        }

        Board copy = *this;
        auto currMove = m_history.rbegin();


        auto revertMoves = [&] {
            ASSERT(!copy.m_history.empty());
            ASSERT(currMove->performedMove == copy.m_history.back().performedMove);
            copy.undoMove();
            ++currMove;
            ASSERT(!copy.m_history.empty());
            ASSERT(currMove->performedMove == copy.m_history.back().performedMove);
            copy.undoMove();
            ++currMove;
        };


        revertMoves();
        ASSERT(currMove != m_history.rend());

        for (uint32_t moveIndex = 4; moveIndex <= maxMoves; moveIndex += 2) {
            revertMoves();

            if (copy.m_castlingRights != m_castlingRights || copy.m_enPassant.has_value()) {
                // castling and en passant do not reset directly but do break repetition
                break;
            }

            if (copy.m_pieces == m_pieces) {
                return copy.m_repeated + 1;
            }
        }

        return 0;
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
