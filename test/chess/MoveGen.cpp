#include <catch2/catch.hpp>
#include <chess/MoveGen.h>

TEST_CASE("Move generation", "[chess][movegen]") {
    using namespace Chess;
    Board board = Board::emptyBoard();
    MoveList list = Chess::generateAllMoves(board);
    REQUIRE(list.size() == 0);
}
