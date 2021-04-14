#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/Board.h>

using namespace Chess;

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
}
