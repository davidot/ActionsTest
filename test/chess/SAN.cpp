#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/Board.h>

using namespace Chess;

#define STRING_COLS TEST_SOME(values({"a", "b", "c", "d", "e", "f", "g", "h"}))

TEST_CASE("Basic SAN parsing", "[chess][parsing][san]") {

    Piece filledPiece = Piece::fromFEN('p').value();                  // black pawn
    Piece otherPiece = Piece(Chess::Piece::Type::Queen, Color::White);// white queen
    REQUIRE_FALSE(filledPiece == otherPiece);

    auto b = Board::fromFEN("pppppppp/pppppppp/pppppppp/pppppppp/pppppppp/pppppppp/pppppppp/pppppppp w - - 0 1");
    REQUIRE(b);
    Board filledBoard = b.extract();

    SECTION("Correct squares") {
        auto is_position = [&](uint8_t column, uint8_t row, const std::string &san) {
            // since we do not actually specify the index order we have to test via a Board
            CAPTURE(row, column, san);
            filledBoard.setPiece(column, row, otherPiece);
            REQUIRE(filledBoard.pieceAt(column, row) == otherPiece);
            auto newPiece = filledBoard.pieceAt(san);
            REQUIRE(newPiece);
            REQUIRE(newPiece.value() == otherPiece);
            filledBoard.setPiece(san, std::nullopt);
            REQUIRE(filledBoard.pieceAt(column, row) == std::nullopt);
            filledBoard.setPiece(column, row, filledPiece);

            REQUIRE(Board::SANToColRow(san) == std::make_pair(column, row));
        };

        is_position(0, 0, "a1");
        is_position(1, 0, "b1");
        is_position(0, 1, "a2");
        is_position(2, 1, "c2");
        is_position(3, 3, "d4");
        is_position(7, 7, "h8");
        is_position(7, 0, "h1");

        is_position(4, 2, "e3");
        is_position(6, 4, "g5");
        is_position(5, 5, "f6");
        is_position(6, 6, "g7");
        is_position(6, 1, "g2");
        is_position(5, 2, "f3");
    }

    SECTION("Failing squares") {
        Board empty = Board::emptyBoard();

        auto is_not_a_position = [&](const std::string &san) {
            CAPTURE(san);
            REQUIRE_FALSE(filledBoard.pieceAt(san).has_value());
            empty.setPiece(san, Piece(Piece::Type::Pawn, Color::Black));
            REQUIRE(empty.countPieces(Color::Black) == 0);
            REQUIRE(empty.countPieces(Color::White) == 0);
            REQUIRE_FALSE(Board::SANToColRow(san).has_value());
        };

        is_not_a_position("");
        is_not_a_position("x");

        is_not_a_position("a0");
        is_not_a_position("a9");

        is_not_a_position("h0");
        is_not_a_position("h9");

        is_not_a_position("g0");
        is_not_a_position("g9");

        is_not_a_position("0h");
        is_not_a_position("0a");

        is_not_a_position("1a");
        is_not_a_position("1h");
    }

    SECTION("All squares are valid") {
        Board board = Board::emptyBoard();
        std::string col = GENERATE(TEST_SOME(values({"a", "b", "c", "d", "e", "f", "g", "h"})));
        BoardIndex row = GENERATE(TEST_SOME(range(0, 8)));
        REQUIRE(board.SANToColRow(col + std::to_string(row)).has_value());
    }
}


TEST_CASE("SAN move parsing", "[.][chess][parsing][san][move]") {

    //  file of departure if different
    //  rank of departure if the files are the same but the ranks differ
    //  the complete origin square coordinate otherwise

    // en passant hits on destination square
    // only ambiguity on valid moves!
    // example of pinned piece forcing PGN non ambiguous (Q in row 1)
    // 2k5/3p4/4Q3/8/4q1p1/8/4Q3/4K3 w - - 0 1


    SECTION("Can parse simple pawn move") {
        Board board = Board::standardBoard();

        std::string col = GENERATE(TEST_SOME(values({"a", "b", "c", "d", "e", "f", "g", "h"})));

        auto simplePawnPush = col +"3";
        Move expectedMove{col + "2", col + "3"};

        auto move = board.parseSANMove(simplePawnPush);
        auto san = board.moveToSAN(expectedMove);
        CHECK(san == simplePawnPush);
        CHECK(move == expectedMove);
        REQUIRE(move.has_value());
    }

    SECTION("Can parse double push pawn move") {
        Board board = Board::standardBoard();

        std::string col = GENERATE(TEST_SOME(values({"a", "b", "c", "d", "e", "f", "g", "h"})));

        auto doublePawnPush = col +"4";
        Move expectedMove{col + "2", col + "4", Move::Flag::DoublePushPawn};

        auto move = board.parseSANMove(doublePawnPush);
        auto san = board.moveToSAN(expectedMove);
        CHECK(san == doublePawnPush);
        CHECK(move == expectedMove);
        REQUIRE(move.has_value());
    }

}
