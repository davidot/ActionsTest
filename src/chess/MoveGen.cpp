#include "MoveGen.h"
#include "../util/Assertions.h"
#include "Piece.h"
#include <array>
#include <optional>

#include "BitBoard.h"

namespace Chess {

    using namespace BB;

    void MoveList::addMove(Move move) {
        // not sure we actually want to reject none moves?
        ASSERT(move.fromPosition != move.toPosition);
        m_moves.push_back(move);
    }

    size_t MoveList::size() const {
        return m_moves.size();
    }

    bool MoveList::isStaleMate() const {
        return size() == 0 && !m_inCheck;
    }

    bool MoveList::isCheckMate() const {
        return size() == 0 && m_inCheck;
    }

    void MoveList::kingAttacked() {
        m_inCheck = true;
    }


    constexpr static BoardIndex boardSize = Board::size;

#define ALL_DIRECTIONS LeftUp, Up, RightUp, Left, Right, LeftDown, Down, RightDown

    bool withinRange(BoardIndex v, BoardOffset o) {
        if (o < 0) {
            return v >= -o;
        }
        return (boardSize - v) > o;
    }

    bool validOffset(BoardIndex &col, BoardIndex &row, Offsets offset) {
        if (withinRange(col, offset.first) && withinRange(row, offset.second)) {
            col += offset.first;
            row += offset.second;
            return true;
        }
        return false;
    }

    bool validateMove(MoveList &list, const Board &board, Move m) {
        ASSERT(m.fromPosition != m.toPosition);
        ASSERT(board.pieceAt(m.colRowFromPosition()).has_value() && board.pieceAt(m.colRowFromPosition())->color() == board.colorToMove());

        BitBoard toBB = squareBoard(m.toPosition);
        if (m.flag != Move::Flag::Castling &&
            (toBB & (board.typeBitboard(Piece::Type::King) | board.colorBitboard(board.colorToMove())))) {
            return false;
        }

        if (board.isLegal(m)) {
            list.addMove(m);
        }

        return !(toBB & board.piecesBB);
    }

    template<Direction direction>
    void addMove(MoveList &list, const Board &board, BoardIndex col, BoardIndex row) {
        constexpr auto off = offsets[direction];
        BoardIndex newCol = col;
        BoardIndex newRow = row;
        if (validOffset(newCol, newRow, off)) {
            validateMove(list, board, Move{col, row, newCol, newRow});
        }
    }

    template<Direction... directions>
    void addMoves(MoveList &list, const Board &board, BoardIndex col, BoardIndex row) {
        (addMove<directions>(list, board, col, row), ...);
    }

    template<Direction d>
    void addSlidingMoves(MoveList &list, const Board &board, BoardIndex col, BoardIndex row) {
        BoardIndex toCol = col;
        BoardIndex toRow = row;
        auto &offset = offsets[d];
        while (validOffset(toCol, toRow, offset)) {
            if (!validateMove(list, board, Move{col, row, toCol, toRow})) {
                break;
            }
        }
    }

    template<Direction... Directions>
    void addAllSlidingMoves(MoveList &list, const Board &board, BoardIndex col, BoardIndex row) {
        (addSlidingMoves<Directions>(list, board, col, row), ...);
    }

    void addKnightMoves(MoveList &list, const Board &board, BoardIndex col, BoardIndex row) {
        for (auto &off : knightOffsets) {
            BoardIndex newCol = col;
            BoardIndex newRow = row;
            if (validOffset(newCol, newRow, off)) {
                validateMove(list, board, Move{col, row, newCol, newRow});
            }
        }
    }


    template<Direction direction>
    void addPromotion(MoveList& list, const Board& board, BoardIndex toIndex) {
        for (auto promotion : {Move::Flag::PromotionToKnight,
                               Move::Flag::PromotionToBishop,
                               Move::Flag::PromotionToRook,
                               Move::Flag::PromotionToQueen}) {
            validateMove(list, board, Move{toIndex - indexOffsets[direction], toIndex,  promotion});
        }
    }

    bool empty(const Board &board, BoardIndex colFrom, BoardIndex colTo, BoardIndex row) {
        BoardIndex start = std::min(colFrom, colTo) + 1u;
        BoardIndex end = std::max(colFrom, colTo);
        while (start < end) {
            if (board.pieceAt(start, row) != std::nullopt) {
                return false;
            }
            ++start;
        }

        return true;
    }

    void addCastles(MoveList &list, const Board &board, BoardIndex col, BoardIndex row, Color color) {
        auto rights = board.castlingRights();
        rights = rights & (color == Color::White ? CastlingRight::WhiteCastling : CastlingRight::BlackCastling);
        if (rights == CastlingRight::NoCastling) {
            return;
        }
        auto home = Board::homeRow(color);
        if (home != row || col != Board::kingCol) {
            // this is technically not valid since we have castling rights but solves things for multiple kings...
            return;
        }
        auto addCastleMove = [&](CastlingRight required, BoardIndex rookCol) {
            if ((rights & required) != CastlingRight::NoCastling && empty(board, Board::kingCol, rookCol, home) && board.pieceAt(rookCol, home) == Piece{Piece::Type::Rook, color}) {
                validateMove(list, board, Move{Board::kingCol, home, rookCol, home, Move::Flag::Castling});
            }
        };
        addCastleMove(CastlingRight::KingSideCastling, Board::kingSideRookCol);
        addCastleMove(CastlingRight::QueenSideCastling, Board::queenSideRookCol);
    }

    template<Color color>
    void generatePawnMoves(BitBoard pawns, const Board& board, MoveList& list, BitBoard us, BitBoard them, std::optional<BitBoard> epBB) {
        constexpr BitBoard doublePushRow = color == Color::White ? row2 : row5;
        constexpr BitBoard promoRow = color == Color::White ? row6 : row1;
        constexpr Direction Forward = Board::pawnDirection(color) > 0 ? Up : Down;
        constexpr Direction Backward = Board::pawnDirection(color) > 0 ? Down : Up;
        constexpr BoardIndex Back = indexOffsets[Backward];
        constexpr auto LeftForward = static_cast<Direction>(Forward + ToLeft);
        constexpr auto RightForward = static_cast<Direction>(Forward + ToRight);

        BitBoard empty = ~(us | them);

        BitBoard promoRowPawn = pawns & promoRow;
        const BitBoard otherPawns = pawns & ~promoRow;

        BitBoard push = shift<Forward>(otherPawns) & empty;
        BitBoard doublePush = shift<Forward>(push & doublePushRow) & empty;

        while (push) {
            BoardIndex index = popLsb(push);
            validateMove(list, board, Move(index + Back, index));
        }

        while (doublePush) {
            BoardIndex index = popLsb(doublePush);
            validateMove(list, board, Move(index + Back + Back, index, Move::Flag::DoublePushPawn));
        }

        if (promoRowPawn) {
            BitBoard captureLeft = shift<LeftForward>(promoRowPawn) & them;
            BitBoard captureRight = shift<RightForward>(promoRowPawn) & them;
            BitBoard move = shift<Forward>(promoRowPawn) & empty;

            while (captureLeft) {
                BoardIndex index = popLsb(captureLeft);
                addPromotion<LeftForward>(list, board, index);
            }

            while (captureRight) {
                BoardIndex index = popLsb(captureRight);
                addPromotion<RightForward>(list, board, index);
            }

            while (move) {
                BoardIndex index = popLsb(move);
                addPromotion<Forward>(list, board, index);
            }
        }

        BitBoard captureLeft = shift<LeftForward>(otherPawns) & them;
        BitBoard captureRight = shift<RightForward>(otherPawns) & them;

        while (captureLeft) {
            BoardIndex index = popLsb(captureLeft);
            validateMove(list, board, Move(index - indexOffsets[LeftForward], index));
        }

        while (captureRight) {
            BoardIndex index = popLsb(captureRight);
            validateMove(list, board, Move(index - indexOffsets[RightForward], index));
        }

        if (epBB.has_value()) {
            BitBoard epSquare = *epBB;
            if (shift<static_cast<Direction>(Backward + ToLeft)>(epSquare) & pawns) {
                BoardIndex epIndex = popLsb(epSquare);
                validateMove(list, board, Move(epIndex + indexOffsets[Backward + ToLeft], epIndex, Move::Flag::EnPassant));
            }

            epSquare = *epBB;
            if (shift<static_cast<Direction>(Backward + ToRight)>(epSquare) & pawns) {
                BoardIndex epIndex = popLsb(epSquare);
                validateMove(list, board, Move(epIndex + indexOffsets[Backward + ToRight], epIndex, Move::Flag::EnPassant));
            }
        }


    }

    MoveList generateAllMoves(const Board &board) {
#ifdef OUTPUT_FEN
        std::cout << board.toFEN() << '\n';
#endif
        MoveList list{};
        Color color = board.colorToMove();

        ASSERT((board.colorPiecesBB[0] | board.colorPiecesBB[1]) == board.piecesBB);
        ASSERT((board.piecesBB ^ board.colorPiecesBB[0]) == board.colorPiecesBB[1]);
        ASSERT((board.piecesBB ^ board.colorPiecesBB[1]) == board.colorPiecesBB[0]);

        BitBoard pieces = board.colorBitboard(color);
        BitBoard them = board.colorBitboard(opposite(color));

        {
            BitBoard pawns = pieces & board.typeBitboard(Piece::Type::Pawn);

            if (color == Color::White) {
                generatePawnMoves<Color::White>(pawns, board, list, pieces, them, board.enPassantBB());
            } else {
                generatePawnMoves<Color::Black>(pawns, board, list, pieces, them, board.enPassantBB());
            }

            pieces &= ~pawns;
        }

        while (pieces != 0) {
            BoardIndex index = popLsb(pieces);

            ASSERT(board.pieceAt(index).has_value());
            auto piece = board.pieceAt(index).value();

            auto [col, row] = Board::indexToColumnRow(index);

            switch (piece.type()) {
                case Piece::Type::King:
                    addMoves<ALL_DIRECTIONS>(list, board, col, row);
                    addCastles(list, board, col, row, color);
                    break;
                case Piece::Type::Knight:
                    addKnightMoves(list, board, col, row);
                    break;
                case Piece::Type::Bishop:
                    addAllSlidingMoves<LeftUp, RightUp, LeftDown, RightDown>(list, board, col, row);
                    break;
                case Piece::Type::Rook:
                    addAllSlidingMoves<Up, Left, Right, Down>(list, board, col, row);
                    break;
                case Piece::Type::Queen:
                    addAllSlidingMoves<ALL_DIRECTIONS>(list, board, col, row);
                    break;
                case Piece::Type::Pawn:
                default:
                    ASSERT_NOT_REACHED();
            }

        }

        if (list.size() == 0) {
            // add king check to differentiate check and stale mate
            auto [kingCol, kingRow] = board.kingSquare(board.colorToMove());
            if (board.attacked(kingCol, kingRow)) {
                list.kingAttacked();
            }
        }

        return list;
    }

    bool MoveList::contains(Move move) const {
        return hasMove([&move](const Move& mv) {
          return mv == move;
        });
    }
}// namespace Chess
