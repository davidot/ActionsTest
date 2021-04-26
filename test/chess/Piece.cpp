#include "TestUtil.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <chess/Piece.h>
#include <chess/Move.h>

TEST_CASE("Pieces", "[chess][base]") {

    using namespace Chess;
    using Pt = Piece::Type;

#define ALL_TYPES TEST_SOME(values({Pt::Pawn, Pt::Rook, Pt::Knight, Pt::Bishop, Pt::Queen, Pt::King}))
#define ALL_COLORS Color::White, Color::Black

    SECTION("Can create all piece and read/write them from FEN") {
        Piece piece(GENERATE(ALL_TYPES), GENERATE(ALL_COLORS));

        CAPTURE(piece);

        REQUIRE(piece == piece);
        REQUIRE_FALSE(piece != piece);

        Piece::IntType intVal = piece.toInt();
        REQUIRE(intVal >= 0);

        char fenVal = piece.toFEN();
        REQUIRE(std::isalpha(fenVal));

        CAPTURE(fenVal, intVal);

        // Can convert back
        REQUIRE(piece == Piece::fromFEN(fenVal));
        REQUIRE(intVal == Piece::intFromFEN(fenVal));
        REQUIRE(piece == Piece::fromInt(intVal));
        REQUIRE(Piece::isPiece(intVal));
    }

    SECTION("No two pieces can have the same FEN or INT value") {
        Piece piece1(GENERATE(ALL_TYPES), GENERATE(ALL_COLORS));
        Piece piece2(GENERATE(ALL_TYPES), GENERATE(ALL_COLORS));

        if (piece1 == piece2) {
            REQUIRE(piece1.toInt() == piece2.toInt());
            REQUIRE(piece1.toFEN() == piece2.toFEN());
            REQUIRE(piece1.toUTF8Char() == piece2.toUTF8Char());
        } else {
            REQUIRE(piece1 != piece2);
            CAPTURE(piece1, piece2);

            REQUIRE(piece1.toInt() != piece2.toInt());
            REQUIRE(piece1.toFEN() != piece2.toFEN());
            REQUIRE(piece1.toUTF8Char() != piece2.toUTF8Char());
        }
    }

    // some hacky stuff to not have to hardcode the color mask here
    Piece::IntType colorMask = 0u
            | (Piece(Pt::Pawn, Color::White).toInt() & Piece(Pt::Knight, Color::White).toInt())
            | (Piece(Pt::Pawn, Color::Black).toInt() & Piece(Pt::Knight, Color::Black).toInt());

    SECTION("Pieces can be validated") {

        // basic tests
        REQUIRE_FALSE(Piece::isPiece(0));
        REQUIRE_FALSE(Piece::isPiece(Piece::noneValue()));
        REQUIRE_FALSE(Piece::isPiece(~colorMask));
        REQUIRE_FALSE(Piece::isPiece(colorMask));

        Piece::IntType type = GENERATE(range(0b0000, 0b1111));
        Piece::IntType topBits = GENERATE(range(0b00u, 0b11u)) << 6u;

        // note: not actually valid pieces but not things it should check
        SECTION("Generated valid values") {
            Piece::IntType col = GENERATE(0b10000, 0b100000);
            REQUIRE(Piece::isPiece(topBits | col | type));
        }

        SECTION("Generated wrong values") {
            Piece::IntType wrongColor = GENERATE(0b000000, 0b110000);
            REQUIRE_FALSE(Piece::isPiece(topBits | wrongColor | type));
        }
    }
}

TEST_CASE("Basic funcs", "[chess][base]") {
    using namespace Chess;
    Color white = Color::White;
    Color black = Color::Black;
    REQUIRE(opposite(white) == black);
    REQUIRE(opposite(black) == white);
}
