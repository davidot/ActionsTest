#include <catch2/catch.hpp>
#include <chess/MoveGen.h>

#define BENCHMARK_TAGS "[.][chess][benchmark]"

using namespace Chess;

TEST_CASE("Board FEN parsing", "[fen]" BENCHMARK_TAGS) {
    BENCHMARK("Loading standard board") {
        return Board::standardBoard();
    };

    BENCHMARK("Loading empty board") {
        return Board::emptyBoard();
    };

    BENCHMARK("Loading some FEN") {
        auto eb = Board::fromFEN("4Q2Q/4r3/6n1/1bbK1krn/RR1RRnRR/2qn1R1n/4n1nN/Q3Q3 w - - 1 2");
        REQUIRE(eb);
        return eb.extract();
    };
}

TEST_CASE("MoveGen benchmarks", "[movegen]" BENCHMARK_TAGS) {
#define TEST_FEN(fen, note) \
    { \
        Board b = Board::fromFEN(fen).extract(); \
        BENCHMARK("Moves from FEN " note) { \
            return generateAllMoves(b); \
        }; \
    }                 \

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

    TEST_FEN("k7/2QQQQQQ/KQ1QQQQQ/QQQ1QQQQ/QQQQ1QQQ/QQQQQ1QQ/QQQQQQ1Q/QQQQQQQ1 w - - 0 1", "Lost of queens");

    TEST_FEN("kqQQqQqq/qq5Q/Q6q/q6Q/Q6q/Q6q/Q5QQ/qQqqQqQK w - - 0 1", "Many queens both W");
    TEST_FEN("kqQQqQqq/qq5Q/Q6q/q6Q/Q6q/Q6q/Q5QQ/qQqqQqQK b - - 0 1", "Many queens both B");

    TEST_FEN("1n1r1b1r/P1P1P1P1/2BNq1k1/7R/3Q4/1P1N2K1/P1PBP3/5R2 w - - 15 45", "Legal pos many moves");


}