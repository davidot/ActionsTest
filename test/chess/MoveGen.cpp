#include <catch2/catch.hpp>
#include <chess/MoveGen.h>

TEST_CASE("Move generation", "[chess][movegen]") {
    using namespace Chess;

    SECTION("Empty board has no moves") {
        Board board = Board::emptyBoard();
        MoveList list = Chess::generateAllMoves(board);
        REQUIRE(list.size() == 0);
    }

    SECTION("Board with only colors not in turn has no moves") {
        Board board = Board::emptyBoard();
        Color c = opposite(board.colorToMove());
        Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), c};
        board.setPiece(4, 4, p);
        MoveList list = Chess::generateAllMoves(board);
        REQUIRE(list.size() == 0);
    }

    SECTION("Invalid boards still generate moves") {

        SECTION("Board with single pawn has just one move") {
            Board board = Board::emptyBoard();
            board.setPiece(4, 4, Piece(Piece::Type::Pawn, Color::White));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 1);
        }

        SECTION("Board with multiple pawns has multiple moves") {
            uint8_t count = GENERATE(range(2u, 8u));
            Board board = Board::emptyBoard();
            for (uint8_t i = 0; i < count; i++) {
                board.setPiece(i, 4, Piece(Piece::Type::Pawn, Color::White));
            }
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == count);
        }

        SECTION("Board with single rook always has 14 moves") {
            uint8_t index = GENERATE(range(0u, 64u));
            Board board = Board::emptyBoard();
            for (uint8_t i = 0; i < index; i++) {
                board.setPiece(index, Piece(Piece::Type::Rook, Color::White));
            }
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == index);
        }
    }
}
