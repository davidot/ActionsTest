#include "MoveGen.h"


namespace Chess {
    using Index = Board::BoardIndex;
    using Offset = Move::BoardOffset;


    void MoveList::addMove(Move) {
        m_count++;
    }

    void MoveList::addMove(Board::BoardIndex col, Board::BoardIndex row, Move::BoardOffset offset, Move::Flags flags) {
        addMove({col, row, offset, flags});
    }

    size_t MoveList::size() const {
        return m_count;
    }

    constexpr static Index boardSize = 8;

    using Offsets = std::pair<Offset, Offset>;

    constexpr static Offsets offsets[] = {
            {-1, -1}, { 0, -1}, { 1, -1},
            {-1,  0}, { 0,  0}, { 1,  0},
            {-1,  1}, { 0,  1}, { 1,  1},
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
        RightDown = 8
    };

#define ALL_DIRECTIONS LeftUp, Up, RightUp, Left, Right, LeftDown, Down, RightDown

    void applyOffset(Index& col, Index& row, Offsets offset) {
        col += offset.first;
        row += offset.second;
    }

    bool withinRange(Index v, Offset o) {
        if (o < 0) {
            return v >= -o;
        }
        return (boardSize - v) > o;
    }

    bool validOffset(Index col, Index row, Offsets offset) {
        return withinRange(col, offset.first) && withinRange(row, offset.second);
    }

    template<Direction direction>
    void addMove(MoveList &list, Index col, Index row) {
        constexpr auto off = offsets[direction];
        if (validOffset(col, row, off)) {
            list.addMove(Move{col, row,
                              static_cast<Index>(col + off.first), static_cast<Index>(row + off.second)});
        }
    }

    template<Direction ...directions>
    void addMoves(MoveList& list, Index col, Index row) {
        (addMove<directions>(list, col, row), ...);
    }

    template<Direction d>
    void addSlidingMoves(MoveList &list, Index col, Index row) {
        Index toCol = col;
        Index toRow = row;
        auto& offset = offsets[d];
        while (validOffset(toCol, toRow, offset)) {
            applyOffset(toCol, toRow, offset);
            list.addMove(Move{col, row, toCol, toRow});
        }
    }

    template<Direction... Directions>
    void addAllSlidingMoves(MoveList &list, Index col, Index row) {
        (addSlidingMoves<Directions>(list, col, row), ...);
    }

    void addKnightMoves(MoveList &list, Index col, Index row) {
        for (auto& off : knightOffsets) {
            if (validOffset(col, row, off)) {
                list.addMove(Move{col, row,
                                  static_cast<Index>(col + off.first), static_cast<Index>(row + off.second)});
            }
        }
    }

    MoveList generateAllMoves(const Board &board) {
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
                        if (color == Color::White) {
                            addMove<Up>(list, col, row);
                        } else {
                            addMove<Down>(list, col, row);
                        }
                        break;
                    case Piece::Type::King:
                        addMoves<ALL_DIRECTIONS>(list, col, row);
                        break;
                    case Piece::Type::Knight:
                        addKnightMoves(list, col, row);
                        break;
                    case Piece::Type::Bishop:
                        addAllSlidingMoves<LeftUp, RightUp, LeftDown, RightDown>(list, col, row);
                        break;
                    case Piece::Type::Rook:
                        addAllSlidingMoves<Up, Left, Right, Down>(list, col, row);
                        break;
                    case Piece::Type::Queen:
                        addAllSlidingMoves<ALL_DIRECTIONS>(list, col, row);
                        break;
                }
            }
        }
        return list;
    }
}// namespace Chess
