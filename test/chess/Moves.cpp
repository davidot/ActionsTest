#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/Board.h>

TEST_CASE("Apply moves to board", "[chess][move]") {
    using namespace Chess;

    Board board = Board::emptyBoard();

    SECTION("No undo on empty board") {
        REQUIRE_FALSE(board.undoMove());
    }

    SECTION("Queen can move from anywhere to anywhere else") {
        uint8_t fromCol = GENERATE(TEST_SOME(range(0, 8)));
        uint8_t fromRow = GENERATE(TEST_SOME(range(0, 8)));

        uint8_t toCol = GENERATE(TEST_SOME(range(0, 8)));
        uint8_t toRow = GENERATE_COPY(
                filter([=](uint8_t r) { return fromCol != toCol || fromRow != r; },
                       TEST_SOME(range(0, 8))));

        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        const Piece piece = Piece{Piece::Type::Queen, c};
        board.setPiece(fromCol, fromRow, piece);

        // TODO this actually generates illegal moves so maybe clamp down on that
        auto madeMove = board.makeMove(Move{fromCol, fromRow, toCol, toRow});
        REQUIRE(madeMove);
        REQUIRE(board.pieceAt(toCol, toRow) == piece);
        REQUIRE_FALSE(board.pieceAt(fromCol, fromRow).has_value());

        SECTION("Undo move") {
            REQUIRE(board.undoMove());
            REQUIRE_FALSE(board.pieceAt(toCol, toRow).has_value());
            REQUIRE(board.pieceAt(fromCol, fromRow) == piece);
        }
    }

    SECTION("Can move pawn in right direction") {
        uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
        uint8_t fromRow = GENERATE(TEST_SOME(range(2, 6)));


        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        uint8_t toRow = fromRow + Board::pawnDirection(c);

        const Piece piece = Piece{Piece::Type::Pawn, c};
        board.setPiece(col, fromRow, piece);
        auto madeMove = board.makeMove(Move{col, fromRow, col, toRow});
        REQUIRE(madeMove);
        REQUIRE(board.pieceAt(col, toRow) == piece);
        REQUIRE_FALSE(board.pieceAt(col, fromRow).has_value());

        SECTION("Undo move") {
            REQUIRE(board.undoMove());
            REQUIRE_FALSE(board.pieceAt(col, toRow).has_value());
            REQUIRE(board.pieceAt(col, fromRow) == piece);
        }
    }

#define CAPTURABLE_TYPES TEST_SOME(values({Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen}))

    SECTION("Can capture another piece") {
        uint8_t fromCol = 1;
        uint8_t fromRow = 1;

        uint8_t toCol = 1;
        uint8_t toRow = 2;

        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        const Piece piece = Piece{Piece::Type::Queen, c};
        const Piece capturing = Piece{GENERATE(CAPTURABLE_TYPES), opposite(c)};

        board.setPiece(fromCol, fromRow, piece);
        board.setPiece(toCol, toRow, capturing);

        auto madeMove = board.makeMove(Move{fromCol, fromRow, toCol, toRow});
        REQUIRE(madeMove);
        REQUIRE(board.pieceAt(toCol, toRow) == piece);
        REQUIRE_FALSE(board.pieceAt(fromCol, fromRow).has_value());

        SECTION("Undo places exact piece back") {
            REQUIRE(board.undoMove());
            REQUIRE(board.pieceAt(toCol, toRow) == capturing);
            REQUIRE(board.pieceAt(fromCol, fromRow) == piece);
        }

    }
}


// TODO test PGN generation
// example of pinned piece forcing PGN non ambiguous
// 2k5/3p4/4Q3/8/4q1p1/8/4Q3/4K3 w - - 0 1
