#pragma once

#include "Board.h"
#include "Types.h"
#include <array>
namespace Chess::BB {

    using Offsets = std::pair<BoardOffset, BoardOffset>;

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

    constexpr static BoardOffset indexOffsets[] = {
            7,
            8,
            9,
            -1,
            0,
            1,
            -9,
            -8,
            -7};

    constexpr static BoardOffset knightIndexOffsets[] = {
            15, 17, 6, 10,
            -15, -17, -6, -10};

    // TODO: unify all knight offsets somewhere
    constexpr static std::array<Offsets, 8> knightOffsets = {{{-2, -1},
                                                              {-1, -2},
                                                              {1, -2},
                                                              {2, -1},
                                                              {-2, 1},
                                                              {-1, 2},
                                                              {1, 2},
                                                              {2, 1}}};

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


    template<Color c>
    BitBoard pawnAttacksBB(BoardIndex square);

    BitBoard pawnAttackBB(Color c, BoardIndex square);

    template<Piece::Type PieceType>
    BitBoard pieceAttacksBB(BoardIndex square);

    BitBoard pieceAttacksBB(Piece::Type tp, BoardIndex square);

    template<Piece::Type PieceType>
    BitBoard pieceAttacksOccupied(BoardIndex square, BitBoard occupied);

    constexpr BitBoard col0 = 0x0101010101010101ull;
    constexpr BitBoard col7 = col0 << 7;

    constexpr BitBoard row0 = 0xff;
    constexpr BitBoard row1 = row0 << (1 * 8);
    constexpr BitBoard row2 = row0 << (2 * 8);
    constexpr BitBoard row3 = row0 << (3 * 8);
    constexpr BitBoard row4 = row0 << (4 * 8);
    constexpr BitBoard row5 = row0 << (5 * 8);
    constexpr BitBoard row6 = row0 << (6 * 8);
    constexpr BitBoard row7 = row0 << (7 * 8);


    template<Direction dir>
    inline BitBoard shift(BitBoard bb) {
        switch (dir) {
            case Up:
                return bb << 8;
            case Down:
                return bb >> 8;
            case Left:
                return (bb & ~col0) >> 1;
            case Right:
                return (bb & ~col7) << 1;
            case LeftUp:
                return (bb & ~col0) << 7;
            case LeftDown:
                return (bb & ~col0) >> 9;
            case RightUp:
                return (bb & ~col7) << 9;
            case RightDown:
                return (bb & ~col7) >> 7;
            case Middle:
                return bb;
        }
    }

    constexpr BitBoard squareBoard(BoardIndex i) {
        return 1ull << i;
    }

    inline BoardIndex popLsb(BitBoard& bb) {
        //        ASSERT(bb != 0);
        BitBoard lsb = bb & (~bb + 1);
        BoardIndex bi = 0;
        while (lsb != 0) {
            ++bi;
            lsb >>= 1;
        }
        bb &= bb - 1;
        return bi - 1;
    }

    inline bool moreThanOne(BitBoard bb) {
        return bb & (bb - 1);
    }

    inline BoardIndex countBits(BitBoard bb) {
        BoardIndex count = 0;
        while (bb) {
            count += bb & 1;
            bb >>= 1;
        }
        return count;
    }

    BitBoard between(BoardIndex a, BoardIndex b);

    template<Piece::Type tp>
    BitBoard generateSliders(BoardIndex from, BitBoard stoppers);

    bool aligned(BoardIndex a, BoardIndex b, BoardIndex c);

    std::string printBB(BitBoard);
}// namespace Chess::BB
