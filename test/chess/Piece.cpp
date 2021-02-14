#include <catch2/catch.hpp>
#include <chess/Piece.h>

TEST_CASE("Pieces") {

    using namespace Chess;
    using Pt = Piece::Type;
    using Pc = Piece::Color;




#define ALL_TYPES Pt::Pawn, Pt::Rook, Pt::Knight, Pt::Bishop, Pt::Queen, Pt::King
#define ALL_COLORS Pc::White, Pc::Black


    SECTION("Can create all piece") {
        Piece piece(GENERATE(ALL_TYPES), GENERATE(ALL_COLORS));

        REQUIRE(piece == piece);

        uint16_t intVal = piece.toInt();
        REQUIRE(intVal >= 0);

        char fenVal = piece.toFEN();
        REQUIRE(std::isalpha(fenVal));

        // Can convert back

        Piece fromFEN = Piece::fromFEN(fenVal);
        REQUIRE(piece == fromFEN);
        REQUIRE(intVal == Piece::intFromFEN(fenVal));
    }


}
