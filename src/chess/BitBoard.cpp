#include "BitBoard.h"
#include "../util/Assertions.h"
#include <bitset>

#include <climits>
#if CHAR_BIT != 8
#error Please use 8 bit wide char/bytes
#endif


namespace Chess::BB {

    constexpr BoardIndex boardSize = Board::size * Board::size;

    constexpr std::array<std::array<BoardIndex, boardSize>, boardSize> computeDistance() {
        auto split = [](BoardIndex i) {
            return std::make_pair(i & 0b111u, i >> 3u);
        };

        auto diff = [](BoardIndex a, BoardIndex b) -> BoardIndex {
            if (a > b) {
                return a - b;
            }
            // b >= a
            return b - a;
        };

        auto d = [&diff, &split](BoardIndex a, BoardIndex b) {
            auto [a1, a2] = split(a);
            auto [b1, b2] = split(b);
            return std::max(diff(a1, b1), diff(a2, b2));
        };
        std::array<std::array<BoardIndex, boardSize>, boardSize> distances{};
        for (BoardIndex i = 0; i < boardSize; ++i) {
            for (BoardIndex j = 0; j < boardSize; ++j) {
                distances[i][j] = d(i, j);
                //                distances[j][i] = distances[i][j];
            }
        }
        return distances;
    }

    constexpr static auto squareDistances = computeDistance();


    constexpr int typeIndex(Piece::Type tp) {
        return static_cast<int>(tp) - 2;
    }

    constexpr int colorIndex(Color c) {
        return c == Color::Black;
    }

    static_assert(typeIndex(Piece::Type::None) < 0);
    static_assert(typeIndex(Piece::Type::Pawn) < 0);
    static_assert(typeIndex(Piece::Type::King) >= 0);

    static std::array<std::array<BitBoard, Board::size * Board::size>, 2> pawnAttacks{};

    template<Color c>
    BitBoard generatePawnMove(BitBoard square) {
        constexpr Direction Forward = Board::pawnDirection(c) > 0 ? Up : Down;
        constexpr auto LeftForward = static_cast<Direction>(Forward + ToLeft);
        constexpr auto RightForward = static_cast<Direction>(Forward + ToRight);
        return shift<LeftForward>(square) | shift<RightForward>(square);
    }

    template<Color c>
    BitBoard pawnAttacksBB(BoardIndex square) {
        constexpr int cc = colorIndex(c);
        return pawnAttacks[cc][square];
    }

    template BitBoard pawnAttacksBB<Color::White>(BoardIndex);
    template BitBoard pawnAttacksBB<Color::Black>(BoardIndex);

    BitBoard pawnAttackBB(Color c, BoardIndex square) {
        if (c == Color::White) {
            return pawnAttacksBB<Color::White>(square);
        }
        return pawnAttacksBB<Color::Black>(square);
    }

    constexpr BitBoard nonWrapping(BoardIndex from, BoardOffset jump) {
        BoardIndex to = from + jump;
        if (to >= boardSize || squareDistances[from][to] > 2) {
            return 0;
        }
        return squareBoard(to);
    }

    template<Piece::Type tp>
    BitBoard generateSliders(BoardIndex from, BitBoard stoppers) {
        static_assert(tp == Piece::Type::Bishop || tp == Piece::Type::Rook);

        stoppers &= ~squareBoard(from);

        BitBoard attacking = 0;
        constexpr std::array<Direction, 4> BishopDirections = {LeftUp, RightUp, LeftDown, RightDown};
        constexpr std::array<Direction, 4> RookDirections = {Up, Left, Right, Down};
        constexpr auto Directions = (tp == Piece::Type::Bishop) ? BishopDirections : RookDirections;

        for (auto dir : Directions) {
            BoardIndex to = from;
            while (nonWrapping(to, indexOffsets[dir]) && !(stoppers & squareBoard(to))) {
                to += indexOffsets[dir];
                attacking |= squareBoard(to);
            }
        }

        return attacking;
    }

    template BitBoard generateSliders<Piece::Type::Bishop>(BoardIndex from, BitBoard stoppers);
    template BitBoard generateSliders<Piece::Type::Rook>(BoardIndex from, BitBoard stoppers);

    template<>
    BitBoard generateSliders<Piece::Type::Queen>(BoardIndex from, BitBoard stoppers) {
        return generateSliders<Piece::Type::Bishop>(from, stoppers) | generateSliders<Piece::Type::Rook>(from, stoppers);
    }


    std::string printBB(BitBoard bb) {
        std::string base = std::bitset<64>(bb).to_string();
        ASSERT(base.size() == 64);
        for (int i = 8; i < 64; i += 9) {
            base.insert(i, 1, '\n');
        }
        return base;
    }

    static std::array<std::array<BitBoard, Board::size * Board::size>, Piece::pieceTypes - 1> pseudoAttacks{};

    template<Piece::Type PieceType>
    BitBoard pieceAttacksBB(BoardIndex square) {
        constexpr int ti = typeIndex(PieceType);
        return pseudoAttacks[ti][square];
    }

    template BitBoard pieceAttacksBB<Piece::Type::King>(BoardIndex);
    template BitBoard pieceAttacksBB<Piece::Type::Knight>(BoardIndex);
    template BitBoard pieceAttacksBB<Piece::Type::Bishop>(BoardIndex);
    template BitBoard pieceAttacksBB<Piece::Type::Rook>(BoardIndex);
    template BitBoard pieceAttacksBB<Piece::Type::Queen>(BoardIndex);

    BitBoard pieceAttacksBB(Piece::Type tp, BoardIndex square) {
        switch (tp) {
            case Piece::Type::King:
                return pieceAttacksBB<Piece::Type::King>(square);
            case Piece::Type::Bishop:
                return pieceAttacksBB<Piece::Type::Bishop>(square);
            case Piece::Type::Rook:
                return pieceAttacksBB<Piece::Type::Rook>(square);
            case Piece::Type::Queen:
                return pieceAttacksBB<Piece::Type::Queen>(square);
            case Piece::Type::Knight:
                return pieceAttacksBB<Piece::Type::Knight>(square);
            default:
                break;
        }
        return 0;
    }

    BitBoard lineBetween(BoardIndex a, BoardIndex b) {
        BitBoard bBB = squareBoard(b);
        for (Piece::Type tp : {Piece::Type::Rook, Piece::Type::Bishop}) {
            if (pieceAttacksBB(tp, a) & bBB) {
                return (pieceAttacksBB(tp, a) & pieceAttacksBB(tp, b)) | squareBoard(a) | bBB;
            }
        }
        return 0;
    }

    BitBoard between(BoardIndex a, BoardIndex b) {
        BitBoard line = lineBetween(a, b);
        BitBoard middle = ((~0llu) << a) ^ ((~0llu) << b);
        BitBoard betweenAndLow = line & middle;
        return betweenAndLow & (betweenAndLow - 1);
    }

    bool aligned(BoardIndex a, BoardIndex b, BoardIndex c) {
        return lineBetween(a, b) & squareBoard(c);
    }

    bool initBB() noexcept {
        using Tp = Piece::Type;
        for (BoardIndex i = 0; i < boardSize; ++i) {

            BitBoard sq = squareBoard(i);
            pawnAttacks[colorIndex(Color::White)][i] = generatePawnMove<Color::White>(sq);
            pawnAttacks[colorIndex(Color::Black)][i] = generatePawnMove<Color::Black>(sq);

            for (auto jump : knightIndexOffsets) {
                pseudoAttacks[typeIndex(Tp::Knight)][i] |= nonWrapping(i, jump);
            }

            for (auto step : {LeftUp, Up, RightUp, Left, Right, LeftDown, Down, RightDown}) {
                pseudoAttacks[typeIndex(Tp::King)][i] |= nonWrapping(i, indexOffsets[step]);
            }

            pseudoAttacks[typeIndex(Tp::Bishop)][i] = generateSliders<Tp::Bishop>(i, 0);
            pseudoAttacks[typeIndex(Tp::Rook)][i] = generateSliders<Tp::Rook>(i, 0);
            pseudoAttacks[typeIndex(Tp::Queen)][i] =
                    pseudoAttacks[typeIndex(Tp::Bishop)][i] | pseudoAttacks[typeIndex(Tp::Rook)][i];
        }

        return true;
    }
    [[maybe_unused]] static bool createdBitBoard = initBB();


}// namespace Chess::BB
