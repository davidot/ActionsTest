#include <catch2/catch.hpp>
#include <chess/MoveGen.h>

TEST_CASE("Move generation", "[chess][movegen]") {
    using namespace Chess;

    SECTION("Empty board has no moves") {
        Board board = Board::emptyBoard();
        MoveList list = Chess::generateAllMoves(board);
        REQUIRE(list.size() == 0);
    }

    SECTION("(Invalid) Board with single piece has just one move") {
        Color col = GENERATE(Color::White, Color::Black);
        Board board = Board::emptyBoard();
        board.setPiece(4, 4, Piece(Piece::Type::Pawn, col));
        MoveList list = Chess::generateAllMoves(board);
        REQUIRE(list.size() == 1);
    }

    SECTION("(Invalid) Board with multiple pieces has multiple moves") {
        uint8_t count = GENERATE(range(2u, 8u));
        Color col1 = GENERATE(Color::White, Color::Black);
        Color col2 = GENERATE(Color::White, Color::Black);
        Board board = Board::emptyBoard();
        for (uint8_t i = 0; i < count; i++) {
            board.setPiece(i, 4, Piece(Piece::Type::Pawn, i & 1 ? col1 : col2));
        }
        MoveList list = Chess::generateAllMoves(board);
        REQUIRE(list.size() == count);
    }

}
