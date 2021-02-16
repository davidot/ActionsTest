#include <catch2/catch.hpp>
#include <chess/Board.h>

TEST_CASE("Board") {

    using namespace Chess;

    SECTION("Empty board has no pieces") {
        uint8_t size = GENERATE(0, 1, 8, 13, 255);
        CAPTURE(size);
        Board b = Board::emptyBoard(size);

        REQUIRE(b.countPieces(GENERATE(Piece::Color::White, Piece::Color::Black)) == 0);
        REQUIRE_FALSE(b.hasValidPosition());
    }

}
