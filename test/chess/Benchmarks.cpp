#include <catch2/catch.hpp>
#include <chess/MoveGen.h>
#include <iostream>

#define BENCHMARK_TAGS "[.][chess][benchmark]"

using namespace Chess;

TEST_CASE("Board FEN parsing", "[fen]" BENCHMARK_TAGS) {
    // we validate it is a valid board first
#define TEST_FEN(fen, note)            \
    {                                  \
        auto eb = Board::fromFEN(fen); \
        REQUIRE(eb);                   \
    }                                  \
    BENCHMARK("Loading FEN: " note) {  \
        return Board::fromFEN(fen);    \
    };

    BENCHMARK("Loading empty board") {
        return Board::emptyBoard();
    };

    BENCHMARK("Loading standard board") {
        return Board::standardBoard();
    };

    TEST_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Start pos");

    TEST_FEN("4Q2Q/4r3/6n1/1bbK1krn/RR1RRnRR/2qn1R1n/4n1nN/Q3Q3 w - - 1 2", "Some FEN");
}
#undef TEST_FEN

TEST_CASE("MoveGen benchmarks", "[movegen]" BENCHMARK_TAGS) {
#define TEST_FEN(fen, note)                      \
    {                                            \
        auto eb = Board::fromFEN(fen);            \
        REQUIRE(eb);                              \
        Board b = eb.extract();\
        BENCHMARK("Moves from FEN " note) {      \
            return generateAllMoves(b);          \
        };                                       \
    }

    {
        Board standard = Board::standardBoard();

        BENCHMARK("Moves from start position") {
            return generateAllMoves(standard);
        };
    }


    // most of these FENs come from:
    // https://www.chess.com/forum/view/fun-with-chess/what-chess-position-has-the-most-number-of-possible-moves

    TEST_FEN("R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1", "Many moves 1");
    TEST_FEN("3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1", "Many moves 2");
    TEST_FEN("8/P1Q4P/6R1/1K1B1N2/3B4/P2N3P/8/1k6 w - - 0 1", "Many moves 3");
    TEST_FEN("8/P4K1P/k1B1N3/4BN1P/1Q6/3R4/P1P1PP1P/6R1 w - - 0 1", "Many moves 4");

    // from interesting board checkmate in 11 full moves forced
    // 4Q2Q/4r3/4n1n1/1bbK1krn/RR1RR1RR/2qn1R1n/4n1nN/Q3Q3 b - - 0 1
    // from https://chess.stackexchange.com/a/33905
    TEST_FEN("4Q2Q/4r3/6n1/1bbK1krn/RR1RRnRR/2qn1R1n/4n1nN/Q3Q3 w - - 1 2", "In check");

    TEST_FEN("k7/2QQQQQQ/KQ1QQQQQ/QQQ1QQQQ/QQQQ1QQQ/QQQQQ1QQ/QQQQQQ1Q/QQQQQQQ1 w - - 0 1", "Lots of queens");

    TEST_FEN("kqQQqQqq/qq5Q/Q6q/q6Q/Q6q/Q6q/Q5QQ/qQqqQqQK w - - 0 1", "Many queens both W");
    TEST_FEN("kqQQqQqq/qq5Q/Q6q/q6Q/Q6q/Q6q/Q5QQ/qQqqQqQK b - - 0 1", "Many queens both B");

    TEST_FEN("1n1r1b1r/P1P1P1P1/2BNq1k1/7R/3Q4/1P1N2K1/P1PBP3/5R2 w - - 15 45", "Legal pos many moves");

    TEST_FEN("RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr w - - 0 1", "Inverted start pos");
}
#undef TEST_FEN

template<bool output = false>
uint64_t countMoves(Board& board, int depth) {
    if (depth <= 0) {
        return 1;
    }
    MoveList moves = generateAllMoves(board);
    if (depth == 1) {
        if constexpr (output) {
            moves.forEachMove([&](const Move& move) {
              std::cout << move.toSANSquares() << ": 1\n";
            });
        }
        return moves.size();
    }
    uint64_t count = 0;
    moves.forEachMove([&](const Move& move) {
        board.makeMove(move);
        uint64_t subCount = countMoves<false>(board, depth - 1);
        if constexpr (output) {
            std::cout << move.toSANSquares() << ": " << subCount << '\n';
        }
        count += subCount;
        board.undoMove();
    });

    return count;
}

TEST_CASE("Perft benchmarks", "[perft][moving]" BENCHMARK_TAGS) {

    // For correct counts see: https://www.chessprogramming.org/Perft_Results

    {
        Board board = Board::standardBoard();

        BENCHMARK("Perft(2) from start position") {
            auto count = countMoves(board, 2);
            REQUIRE(count == 400);
        };

        BENCHMARK("Perft(3) from start position") {
            auto count = countMoves(board, 3);
            REQUIRE(count == 8902);
        };

        BENCHMARK("Perft(4) from start position") {
            auto count = countMoves(board, 4);
            REQUIRE(count == 197281);
        };
#ifdef LONG_BENCHMAKRS
        BENCHMARK("Perft(5) from start position") {
            auto count = countMoves(board, 5);
            REQUIRE(count == 4865609);
        };

        BENCHMARK("Perft(6) from start position") {
            auto count = countMoves(board, 6);
            REQUIRE(count == 119060324llu);
        };
#endif
    }

    {
        Board board = Board::fromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").extract();
        BENCHMARK("Perft(3) from Kiwipete position") {
            auto count = countMoves(board, 3);
            REQUIRE(count == 97862);
        };
#ifdef LONG_BENCHMAKRS
        BENCHMARK("Perft(4) from Kiwipete position") {
            auto count = countMoves(board, 4);
            REQUIRE(count == 4085603);
        };

        BENCHMARK("Perft(5) from Kiwipete position") {
            auto count = countMoves(board, 5);
            REQUIRE(count == 193690690);
        };
#endif

    }

    {
        Board board = Board::fromFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1").extract();
        BENCHMARK("Perft(4) from position 3") {
            auto count = countMoves(board, 4);
            REQUIRE(count == 43238);
        };
#ifdef LONG_BENCHMAKRS
        BENCHMARK("Perft(5) from position 3") {
            auto count = countMoves(board, 5);
            REQUIRE(count == 674624);
        };

        BENCHMARK("Perft(6) from position 3") {
            auto count = countMoves(board, 6);
            REQUIRE(count == 11030083);
        };

        BENCHMARK("Perft(7) from position 3") {
            auto count = countMoves(board, 7);
            REQUIRE(count == 178633661);
        };
#endif
    }

    {
        Board board = Board::fromFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1").extract();
        BENCHMARK("Perft(3) from position 4 W") {
            auto count = countMoves(board, 3);
            REQUIRE(count == 9467);
        };
#ifdef LONG_BENCHMAKRS
        BENCHMARK("Perft(4) from position 4 W") {
            auto count = countMoves(board, 4);
            REQUIRE(count == 422333);
        };

        BENCHMARK("Perft(5) from position 4 W") {
            auto count = countMoves(board, 5);
            REQUIRE(count == 15833292);
        };
#endif
    }

    {
        Board board = Board::fromFEN("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1").extract();
        BENCHMARK("Perft(3) from position 4 B") {
            auto count = countMoves(board, 3);
            REQUIRE(count == 9467);
        };
#ifdef LONG_BENCHMAKRS
        BENCHMARK("Perft(4) from position 4 B") {
            auto count = countMoves(board, 4);
            REQUIRE(count == 422333);
        };

        BENCHMARK("Perft(5) from position 4 B") {
            auto count = countMoves(board, 5);
            REQUIRE(count == 15833292);
        };
#endif
    }

}
