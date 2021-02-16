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
        REQUIRE_FALSE(piece != piece);

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
            REQUIRE(piece1 != piece2);
            CAPTURE(piece1, piece2);

            REQUIRE(piece1.toInt() != piece2.toInt());
            REQUIRE(piece1.toFEN() != piece2.toFEN());
        }
    }

    SECTION("Pieces have correct properties") {
        auto col = GENERATE(ALL_COLORS);


        SECTION("Pawn is not special at all") {
            Piece piece(Pt::Pawn, col);
            CAPTURE(piece);

            REQUIRE_FALSE(piece.canKnightJump());
            REQUIRE_FALSE(piece.canMoveDiagonally());
            REQUIRE_FALSE(piece.canMoveAxisAligned());
            REQUIRE_FALSE(piece.canMoveUnlimited());

            REQUIRE(piece.isPawn());
        }

        SECTION("King can move all around but not unlimited") {
            Piece piece(Pt::King, col);
            CAPTURE(piece);

            REQUIRE_FALSE(piece.canKnightJump());
            REQUIRE_FALSE(piece.isPawn());
            REQUIRE_FALSE(piece.canMoveUnlimited());

            REQUIRE(piece.canMoveAxisAligned());
            REQUIRE(piece.canMoveDiagonally());
        }

        SECTION("Knights can only move in jump") {
            Piece piece(Pt::Knight, col);
            CAPTURE(piece);

            REQUIRE_FALSE(piece.isPawn());
            REQUIRE_FALSE(piece.canMoveUnlimited());
            REQUIRE_FALSE(piece.canMoveAxisAligned());
            REQUIRE_FALSE(piece.canMoveDiagonally());

            REQUIRE(piece.canKnightJump());
        }

        SECTION("Rooks can move axis aligned and unlimited") {
            Piece piece(Pt::Rook, col);
            CAPTURE(piece);

            REQUIRE_FALSE(piece.canKnightJump());
            REQUIRE_FALSE(piece.isPawn());
            REQUIRE_FALSE(piece.canMoveDiagonally());

            REQUIRE(piece.canMoveUnlimited());
            REQUIRE(piece.canMoveAxisAligned());
        }

        SECTION("Bishop can move diagonally and unlimited") {
            Piece piece(Pt::Bishop, col);
            CAPTURE(piece);

            REQUIRE_FALSE(piece.canKnightJump());
            REQUIRE_FALSE(piece.isPawn());
            REQUIRE_FALSE(piece.canMoveAxisAligned());

            REQUIRE(piece.canMoveDiagonally());
            REQUIRE(piece.canMoveUnlimited());
        }

        SECTION("Queen can move diagonally, axis aligned and unlimited") {
            Piece piece(Pt::Queen, col);
            CAPTURE(piece);

            REQUIRE_FALSE(piece.canKnightJump());
            REQUIRE_FALSE(piece.isPawn());

            REQUIRE(piece.canMoveAxisAligned());
            REQUIRE(piece.canMoveDiagonally());
            REQUIRE(piece.canMoveUnlimited());
        }
    }

}
