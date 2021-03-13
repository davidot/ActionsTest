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

        Color toMove = Color::White;
        [[maybe_unused]] Color other = opposite(Color::White); // TODO use or remove
        SECTION("Board with single pawn has just one move") {
            Board board = Board::emptyBoard();
            board.setPiece(4, 4, Piece(Piece::Type::Pawn, toMove));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 1);
        }

        SECTION("Board with multiple pawns has multiple moves") {
            uint8_t count = GENERATE(range(2u, 8u));
            Board board = Board::emptyBoard();
            for (uint8_t i = 0; i < count; i++) {
                board.setPiece(i, 4, Piece(Piece::Type::Pawn, toMove));
            }
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == count);
        }

        SECTION("Board with single rook always has 14 moves") {
            uint8_t index = GENERATE(range(0u, 64u));
            Board board = Board::emptyBoard();
            board.setPiece(index, Piece(Piece::Type::Rook, toMove));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 14);
        }

        SECTION("Bishop in the center has 13 moves") {
            uint8_t col = GENERATE(3, 4);
            uint8_t row = GENERATE(3, 4);
            Board board = Board::emptyBoard();
            board.setPiece(col, row, Piece(Piece::Type::Bishop, toMove));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 13);
        }

        SECTION("Bishop in the corner has 7 moves") {
            uint8_t col = GENERATE(0, 7);
            uint8_t row = GENERATE(0, 7);
            Board board = Board::emptyBoard();
            board.setPiece(col, row, Piece(Piece::Type::Bishop, toMove));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 7);
        }

        SECTION("King in the center has 8 moves") {
            Board board = Board::emptyBoard();
            board.setPiece(4, 4, Piece(Piece::Type::King, toMove));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 8);
        }

        SECTION("Knight in the center has 8 moves") {
            Board board = Board::emptyBoard();
            board.setPiece(4, 4, Piece(Piece::Type::Knight, toMove));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 8);
        }

        SECTION("Queen in the center has 14+13 moves") {
            uint8_t col = GENERATE(3, 4);
            uint8_t row = GENERATE(3, 4);
            Board board = Board::emptyBoard();
            board.setPiece(col, row, Piece(Piece::Type::Queen, toMove));
            MoveList list = Chess::generateAllMoves(board);
            REQUIRE(list.size() == 27);
        }
    }
}
