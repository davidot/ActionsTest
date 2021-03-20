#include "MoveGen.h"
#include "../util/Assertions.h"


namespace Chess {
    using Index = Board::BoardIndex;
    using Offset = Move::BoardOffset;


    void MoveList::addMove(Move move) {
        // not sure we actually want to reject none moves?
        ASSERT(move.fromPosition != move.toPosition);
        m_moves.push_back(move);
    }

    size_t MoveList::size() const {
        return m_moves.size();
    }

    constexpr static Index boardSize = 8;

    using Offsets = std::pair<Offset, Offset>;

    constexpr static Offsets offsets[] = {
            {-1,  1}, { 0,  1}, { 1,  1},
            {-1,  0}, { 0,  0}, { 1,  0},
            {-1, -1}, { 0, -1}, { 1, -1},
    };

    constexpr static Offsets knightOffsets[] = {
            {-2, -1}, {-1, -2}, { 1, -2}, { 2, -1},
            {-2,  1}, {-1,  2}, { 1,  2}, { 2,  1}
    };

    enum Direction {
        LeftUp = 0,
        Up = 1,
        RightUp = 2,
        Left = 3,
        Middle = 4,
        Right = 5,
        LeftDown = 6,
        Down = 7,
        RightDown = 8,

        ToLeft = -1,
        ToRight = +1,
    };

#define ALL_DIRECTIONS LeftUp, Up, RightUp, Left, Right, LeftDown, Down, RightDown

    bool withinRange(Index v, Offset o) {
        if (o < 0) {
            return v >= -o;
        }
        return (boardSize - v) > o;
    }

    bool validOffset(Index& col, Index& row, Offsets offset) {
        if (withinRange(col, offset.first) && withinRange(row, offset.second)) {
            col += offset.first;
            row += offset.second;
            return true;
        }
        return false;
    }

    bool addMoveWithPieceCheck(MoveList& list, const Board& board, Move m) {
        ASSERT(m.fromPosition != m.toPosition);
        auto pieceAtToLocation = board.pieceAt(m.colRowToPosition());
        auto pieceAtFromLocation = board.pieceAt(m.colRowFromPosition());
        ASSERT(pieceAtFromLocation.has_value());
        if (pieceAtToLocation.has_value()) {
            if (pieceAtToLocation->color() != pieceAtFromLocation->color()) {
                list.addMove(m);
            }
            return false;
        } else {
            list.addMove(m);
            return true;
        }
    }

    template<Direction direction>
    void addMove(MoveList &list, const Board& board, Index col, Index row) {
        constexpr auto off = offsets[direction];
        Index newCol = col;
        Index newRow = row;
        if (validOffset(newCol, newRow, off)) {
            addMoveWithPieceCheck(list, board, Move{col, row, newCol, newRow});
        }
    }

    template<Direction ...directions>
    void addMoves(MoveList& list, const Board& board, Index col, Index row) {
        (addMove<directions>(list, board, col, row), ...);
    }

    template<Direction d>
    void addSlidingMoves(MoveList &list, const Board& board, Index col, Index row) {
        Index toCol = col;
        Index toRow = row;
        auto& offset = offsets[d];
        while (validOffset(toCol, toRow, offset)) {
            if (!addMoveWithPieceCheck(list, board, Move{col, row, toCol, toRow})) {
                break;
            }
        }
    }

    template<Direction... Directions>
    void addAllSlidingMoves(MoveList &list, const Board& board, Index col, Index row) {
        (addSlidingMoves<Directions>(list, board, col, row), ...);
    }

    void addKnightMoves(MoveList &list, const Board& board, Index col, Index row) {
        for (auto& off : knightOffsets) {
            Index newCol = col;
            Index newRow = row;
            if (validOffset(newCol, newRow, off)) {
                addMoveWithPieceCheck(list, board, Move{col, row, newCol, newRow});
            }
        }
    }

    Index pawnStartRow(Color color) {
        switch (color) {
            case Color::White:
                return 1;
            case Color::Black:
                return boardSize - 2;
        }
        ASSERT_NOT_REACHED();
    }

    Index pawnPromotionRow(Color color) {
        switch (color) {
            case Color::White:
                return boardSize - 1;
            case Color::Black:
                return 0;
        }
        ASSERT_NOT_REACHED();
    }

    void addPawnMoves(MoveList& list, const Board& board, const Index col, const Index row, Color color) {
        Offset forward = color == Color::White ? Up : Down;

        auto addMove = [col, row, promoRow = pawnPromotionRow(color), &list]
                (Index newCol, Index newRow, Move::Flag flags = Move::Flag::None) {
            if (newRow == promoRow) {
                // we do not want double push and promotion (on 4x4 board which we do not support)
                ASSERT(flags == Move::Flag::None);

                for (auto promotion : {Move::Flag::PromotionToKnight,
                                       Move::Flag::PromotionToBishop,
                                       Move::Flag::PromotionToRook,
                                       Move::Flag::PromotionToQueen}) {
                    list.addMove(Move{col, row, newCol, newRow, promotion});
                }
            } else {
                list.addMove(Move{col, row, newCol, newRow, flags});
            }
        };

        {
            Index newCol = col;
            Index newRow = row;
            if (validOffset(newCol, newRow, offsets[forward])
                && board.pieceAt(newCol, newRow) == std::nullopt) {

                addMove(newCol, newRow);

                if (row == pawnStartRow(color)
                    && validOffset(newCol, newRow, offsets[forward])
                    && board.pieceAt(newCol, newRow) == std::nullopt){
                    // note it is always valid but this changes the position

                    addMove(newCol, newRow, Move::Flag::DoublePushPawn);
                }
            }
        }

        auto [epCol, epRow] = board.enPassantColRow().value_or(std::make_pair(boardSize + 1, boardSize + 1));

        for (auto change : {ToLeft, ToRight}) {
            auto offset = offsets[forward + change];
            Index newCol = col;
            Index newRow = row;
            if (!validOffset(newCol, newRow, offset)) {
                continue;
            }
            auto pieceAt = board.pieceAt(newCol, newRow);
            if (pieceAt.has_value() && pieceAt->color() != color) {
                addMove(newCol, newRow);
            } else if (newCol == epCol && newRow == epRow) {
                addMove(newCol, newRow, Move::Flag::EnPassant);
            }
        }
    }

    const Index queenSideRook = 0;
    const Index kingSideRook = 7;
    const Index kingCol = 4;

    Index homeRow(Color col) {
        return col == Color::White ? 0 : 7;
    }

    bool empty(const Board& board, Index colFrom, Index colTo, Index row) {
        Index start = std::min(colFrom, colTo) + 1u;
        Index end = std::max(colFrom, colTo);
        while (start < end) {
            if (board.pieceAt(start, row) != std::nullopt) {
                return false;
            }
            ++start;
        }

        return true;
    }

    void addCastles(MoveList& list, const Board& board, Index col, Index row, Color color) {
        auto rights = board.castlingRights();
        if (color == Color::White && (rights & CastlingRight::WhiteCastling) == CastlingRight::NoCastling
            || color == Color::Black && (rights & CastlingRight::BlackCastling) == CastlingRight::NoCastling) {
            return;
        }
        auto home = homeRow(color);
        auto addCastleMove = [&](CastlingRight required, Index rookCol) {
          if ((rights & required) != CastlingRight::NoCastling
              && empty(board, kingCol, rookCol, home)
              && board.pieceAt(rookCol, home) == Piece{Piece::Type::Rook, color}) {
              list.addMove(Move{kingCol, home, rookCol, home, Move::Flag::Castling });
          }
        };
        addCastleMove(CastlingRight::KingSideCastling, kingSideRook);
        addCastleMove(CastlingRight::QueenSideCastling, queenSideRook);
    }

    MoveList generateAllMoves(const Board &board, Color color) {
//        std::cout << board.toFEN() << '\n';
        using BI = Board::BoardIndex;

        MoveList list{};

        for (BI col = 0; col < board.size(); col++) {
            for (BI row = 0; row < board.size(); row++) {
                auto opt_piece = board.pieceAt(col, row);
                if (!opt_piece || opt_piece->color() != color) {
                    continue;
                }
                switch (opt_piece->type()) {
                    case Piece::Type::Pawn:
                        addPawnMoves(list, board, col, row, color);
                        break;
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
                }
            }
        }
        return list;
    }
}// namespace Chess
