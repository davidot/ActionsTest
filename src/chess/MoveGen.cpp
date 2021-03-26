#include "MoveGen.h"
#include "../util/Assertions.h"
#include <iostream>
#include <set>


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
            {-1, 1},
            {0, 1},
            {1, 1},
            {-1, 0},
            {0, 0},
            {1, 0},
            {-1, -1},
            {0, -1},
            {1, -1},
    };

    constexpr static Offsets knightOffsets[] = {
            {-2, -1},
            {-1, -2},
            {1, -2},
            {2, -1},
            {-2, 1},
            {-1, 2},
            {1, 2},
            {2, 1}};

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

    bool validOffset(Index &col, Index &row, Offsets offset) {
        if (withinRange(col, offset.first) && withinRange(row, offset.second)) {
            col += offset.first;
            row += offset.second;
            return true;
        }
        return false;
    }

    bool attacked(Index col, Index row, const Board &board, std::vector<std::tuple<Index, Index, bool>> specialSquares) {
        // we assume it is a legal move
        Color us = board.colorToMove();
        Color other = opposite(us);
        auto hasKnight = [&](Index col, Index row) {
            return board.pieceAt(col, row) == Piece{Piece::Type::Knight, other};
        };

        for (auto &off : knightOffsets) {
            if (hasKnight(col + off.first, row + off.second)) {
                return true;
            }
        }

        for (auto &direction : offsets) {
            if (direction.first == 0 && direction.second == 0) {
                continue;
            }

            Index currCol = col;
            Index currRow = row;
            unsigned steps = 0;
            while (validOffset(currCol, currRow, direction)) {
                auto p = board.pieceAt(currCol, currRow);
                auto exception = std::find_if(specialSquares.begin(), specialSquares.end(), [&](auto &tup) {
                    return std::get<0>(tup) == currCol && std::get<1>(tup) == currRow;
                });
                bool empty = !p;

                if (exception != specialSquares.end()) {
                    if (std::get<2>(*exception)) {
                        // true -> ignore me
                        empty = true;
                    } else {
                        // false -> im here
                        break;
                    }
                }
                if (empty) {
                    ASSERT(steps < 9);
                    steps++;
                    continue;
                }
                if (p->color() == us) {
                    break;
                }
                switch (p->type()) {
                    case Piece::Type::Queen:
                        return true;
                    case Piece::Type::Pawn: {
                        Offset forward = other == Color::White ? Down : Up;
                        if (steps == 0 && (direction == offsets[forward + ToLeft] || direction == offsets[forward + ToRight])) {
                            return true;
                        }
                    } break;
                    case Piece::Type::King:
                        if (steps == 0) {
                            return true;
                        }
                        break;
                    case Piece::Type::Bishop:
                        if (direction.first != 0 && direction.second != 0) {
                            return true;
                        }
                        break;
                    case Piece::Type::Rook:
                        if (direction.first == 0 || direction.second == 0) {
                            return true;
                        }
                        break;
                    case Piece::Type::Knight:
                        // covered above
                        break;
                }
                break;
            }
        }

        return false;
    }

    bool validateMove(MoveList &list, const Board &board, Move m) {
        ASSERT(m.fromPosition != m.toPosition);
        auto pieceAtToLocation = board.pieceAt(m.colRowToPosition());
        auto pieceAtFromLocation = board.pieceAt(m.colRowFromPosition());
        ASSERT(pieceAtFromLocation.has_value() && pieceAtFromLocation->color() == board.colorToMove());

        auto isAttacked = [&](Index col, Index row) -> bool {
            if (col >= boardSize || row >= boardSize) {
                return false;
            }
            auto [colFrom, rowFrom] = m.colRowFromPosition();
            auto [colTo, rowTo] = m.colRowToPosition();
            if (m.flag == Move::Flag::EnPassant) {
                ASSERT(board.pieceAt(colTo, rowFrom) == Piece(Piece::Type::Pawn, opposite(board.colorToMove())));
                return attacked(col, row, board, {{colFrom, rowFrom, true}, {colTo, rowTo, false}, {colTo, rowFrom, true}});
            }
            return attacked(col, row, board, {{colFrom, rowFrom, true}, {colTo, rowTo, false}});
        };


        if (m.flag == Move::Flag::Castling) {
            ASSERT(pieceAtToLocation.has_value() && pieceAtToLocation->type() == Piece::Type::Rook && pieceAtFromLocation->type() == Piece::Type::King);
            auto [colFrom, rowFrom] = m.colRowFromPosition();
            auto [colTo, rowTo] = m.colRowToPosition();
            ASSERT(rowFrom == rowTo);
            ASSERT(colFrom != colTo);
            ASSERT(colFrom == 4);
            // we assume empty here for now
            if (colFrom > colTo) {
                // queen side
                Index finalRookSpot = colFrom - 1;
                Index finalKingSpot = colFrom - 2;
                for (Index i = colFrom; i >= finalKingSpot; i--) {
                    if (attacked(i, rowFrom, board, {})) {
                        return false;
                    }
                }
            } else {
                // king side
                Index finalRookSpot = colFrom + 1;
                Index finalKingSpot = colFrom + 2;
                for (Index i = colFrom; i <= finalKingSpot; i++) {
                    if (attacked(i, rowFrom, board, {})) {
                        return false;
                    }
                }
            }

            list.addMove(m);
            // return does not really matter here
            return true;
        }

        if (pieceAtToLocation.has_value() &&
                   (pieceAtToLocation->type() == Piece::Type::King || pieceAtToLocation->color() == pieceAtFromLocation->color())) {
            // if not castling then capturing own piece or king is invalid and should stop sliders
            return false;
        }

        if (pieceAtFromLocation->type() == Piece::Type::King) {
            // king move check final position
            auto [colTo, rowTo] = m.colRowToPosition();
            if (isAttacked(colTo, rowTo)) {
                return false;
            }
        } else {
            // find the king
            if (auto [kingCol, kingRow] = board.kingSquare(board.colorToMove()); isAttacked(kingCol, kingRow)) {
                return true;
            }
        }

        list.addMove(m);
        return !pieceAtToLocation.has_value();
    }

    template<Direction direction>
    void addMove(MoveList &list, const Board &board, Index col, Index row) {
        constexpr auto off = offsets[direction];
        Index newCol = col;
        Index newRow = row;
        if (validOffset(newCol, newRow, off)) {
            validateMove(list, board, Move{col, row, newCol, newRow});
        }
    }

    template<Direction... directions>
    void addMoves(MoveList &list, const Board &board, Index col, Index row) {
        (addMove<directions>(list, board, col, row), ...);
    }

    template<Direction d>
    void addSlidingMoves(MoveList &list, const Board &board, Index col, Index row) {
        Index toCol = col;
        Index toRow = row;
        auto &offset = offsets[d];
        while (validOffset(toCol, toRow, offset)) {
            if (!validateMove(list, board, Move{col, row, toCol, toRow})) {
                break;
            }
        }
    }

    template<Direction... Directions>
    void addAllSlidingMoves(MoveList &list, const Board &board, Index col, Index row) {
        (addSlidingMoves<Directions>(list, board, col, row), ...);
    }

    void addKnightMoves(MoveList &list, const Board &board, Index col, Index row) {
        for (auto &off : knightOffsets) {
            Index newCol = col;
            Index newRow = row;
            if (validOffset(newCol, newRow, off)) {
                validateMove(list, board, Move{col, row, newCol, newRow});
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

    void addPawnMoves(MoveList &list, const Board &board, const Index col, const Index row, Color color) {
        Offset forward = color == Color::White ? Up : Down;

        auto addMove = [col, row, promoRow = pawnPromotionRow(color), &list, &board](Index newCol, Index newRow, Move::Flag flags = Move::Flag::None) {
            if (newRow == promoRow) {
                // we do not want double push and promotion (on 4x4 board which we do not support)
                ASSERT(flags == Move::Flag::None);

                for (auto promotion : {Move::Flag::PromotionToKnight,
                                       Move::Flag::PromotionToBishop,
                                       Move::Flag::PromotionToRook,
                                       Move::Flag::PromotionToQueen}) {
                    validateMove(list, board, Move{col, row, newCol, newRow, promotion});
                }
            } else {
                validateMove(list, board, Move{col, row, newCol, newRow, flags});
            }
        };

        {
            Index newCol = col;
            Index newRow = row;
            if (validOffset(newCol, newRow, offsets[forward]) && board.pieceAt(newCol, newRow) == std::nullopt) {

                addMove(newCol, newRow);

                if (row == pawnStartRow(color) && validOffset(newCol, newRow, offsets[forward]) && board.pieceAt(newCol, newRow) == std::nullopt) {
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

    bool empty(const Board &board, Index colFrom, Index colTo, Index row) {
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

    void addCastles(MoveList &list, const Board &board, Index col, Index row, Color color) {
        auto rights = board.castlingRights();
        if (color == Color::White && (rights & CastlingRight::WhiteCastling) == CastlingRight::NoCastling || color == Color::Black && (rights & CastlingRight::BlackCastling) == CastlingRight::NoCastling) {
            return;
        }
        auto home = homeRow(color);
        if (home != row || col != kingCol) {
            // this is technically not valid since we have castling rights but solves things for multiple kings...
            return;
        }
        auto addCastleMove = [&](CastlingRight required, Index rookCol) {
            if ((rights & required) != CastlingRight::NoCastling && empty(board, kingCol, rookCol, home) && board.pieceAt(rookCol, home) == Piece{Piece::Type::Rook, color}) {
                validateMove(list, board, Move{kingCol, home, rookCol, home, Move::Flag::Castling});
            }
        };
        addCastleMove(CastlingRight::KingSideCastling, kingSideRook);
        addCastleMove(CastlingRight::QueenSideCastling, queenSideRook);
    }

    MoveList generateAllMoves(const Board &board) {
        //        std::cout << board.toFEN() << '\n';
        using BI = Board::BoardIndex;

        MoveList list{};
        Color color = board.colorToMove();

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
