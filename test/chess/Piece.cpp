#include <catch2/catch.hpp>
#include <chess/Piece.h>

TEST_CASE("Pieces") {

    using namespace Chess;
    using Pt = Piece::Type;
    using Pc = Piece::Color;

#define ALL_TYPES Pt::Pawn, Pt::Rook, Pt::Knight, Pt::Bishop, Pt::Queen, Pt::King
#define ALL_COLORS Pc::White, Pc::Black

    SECTION("Can create all piece and read/write them from FEN") {
        Piece piece(GENERATE(ALL_TYPES), GENERATE(ALL_COLORS));

        CAPTURE(piece);

        REQUIRE(piece == piece);

        uint16_t intVal = piece.toInt();
        REQUIRE(intVal >= 0);

        char fenVal = piece.toFEN();
        REQUIRE(std::isalpha(fenVal));

        CAPTURE(fenVal, intVal);

        // Can convert back
        REQUIRE(piece == Piece::fromFEN(fenVal));
        REQUIRE(intVal == Piece::intFromFEN(fenVal));
        REQUIRE(piece == Piece::fromInt(intVal));
    }

    SECTION("No two pieces can have the same FEN or INT value") {
        Piece piece1(GENERATE(ALL_TYPES), GENERATE(ALL_COLORS));
        Piece piece2(GENERATE(ALL_TYPES), GENERATE(ALL_COLORS));

        if (piece1 == piece2) {
            REQUIRE(piece1.toInt() == piece2.toInt());
            REQUIRE(piece1.toFEN() == piece2.toFEN());
        } else {
            CAPTURE(piece1, piece2);

            REQUIRE(piece1.toInt() != piece2.toInt());
            REQUIRE(piece1.toFEN() != piece2.toFEN());
        }
    }


}
