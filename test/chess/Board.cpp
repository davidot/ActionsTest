#include <catch2/catch.hpp>
#include <chess/Board.h>



TEST_CASE("Board", "[chess][base]") {
    using namespace Chess;

#define GENERATE_PIECE() Piece(GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), GENERATE(Color::White, Color::Black))
#define GENERATE_PIECES_WHITE_ONLY() Piece(GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), Color::White)

    SECTION("Empty board has no pieces") {
        uint8_t size = GENERATE(0, 1, 2, 8, 13, 255);
        CAPTURE(size);
        Board b = Board::emptyBoard(size);

        CHECK(b.countPieces(Color::White) == 0);
        CHECK(b.countPieces(Color::Black) == 0);
        CHECK_FALSE(b.hasValidPosition());
        CHECK(b.colorToMove() == Color::White);

        for (uint16_t i = 0; i < size * size; i+= size / 17 + 1u) {
            CAPTURE(i);
            CHECK_FALSE(b.pieceAt(i));
        }
    }

    SECTION("Can add pieces every where") {
        uint32_t size = GENERATE(1, 8, 255);
        Piece piece = GENERATE_PIECE();
        Board b = Board::emptyBoard(size);

        uint32_t index = GENERATE_COPY(0u, size - 1, size * size - 1, size * size, take(3, random(1u, size * size - 1)));

        CAPTURE(size, piece, index);

        b.setPiece(index, piece);

        if (index >= size * size) {
            // out of bounds
            CHECK(b.countPieces(piece.color()) == 0);
        } else {
            CHECK(b.countPieces(piece.color()) == 1);
        }

        // since we only add once piece it can never be a valid position!
        CHECK_FALSE(b.hasValidPosition());

        for (uint32_t i = 0; i <= (size + 1) * size; i+= std::max(1u, size - 2)) {
            CAPTURE(i);
            auto pieceHere = b.pieceAt(i);
            REQUIRE(pieceHere.has_value() == (i == index && index < size * size));
            if (i == index && index < size * size) {
                REQUIRE(*pieceHere == piece);
            }
        }

        SECTION("Remove piece") {
            b.setPiece(index, std::nullopt);

            CHECK(b.countPieces(piece.color()) == 0);
            CHECK_FALSE(b.pieceAt(index).has_value());
        }
    }

    SECTION("Double set piece") {
        uint32_t size = GENERATE(1, 8, 13, 255);
        Piece piece = GENERATE_PIECE();
        Board b = Board::emptyBoard(size);

        uint32_t index = GENERATE_COPY(0u, size - 1, size * size - 1, size * size, take(3, random(1u, size * size - 1)));

        CAPTURE(size, piece, index);

        b.setPiece(index, piece);

        b.setPiece(index, piece);

        if (index >= size * size) {
            // out of bounds
            REQUIRE(b.countPieces(piece.color()) == 0);
        } else {
            REQUIRE(b.countPieces(piece.color()) == 1);
        }

        // since we only add once piece it can never be a valid position!
        REQUIRE_FALSE(b.hasValidPosition());

    }

    SECTION("Remove non-existant piece") {
        uint32_t size = GENERATE(1, 8, 13, 255);
        Board b = Board::emptyBoard(size);

        uint32_t index = GENERATE_COPY(0u, size - 1, size * size - 1, size * size, take(3, random(1u, size * size - 1)));

        CAPTURE(size, index);


        // double set no piece (and should already be no piece of course
        b.setPiece(index, std::nullopt);

        b.setPiece(index, std::nullopt);

        REQUIRE(b.countPieces(Color::White) == 0);
        REQUIRE(b.countPieces(Color::Black) == 0);

        // since we only add once piece it can never be a valid position!
        REQUIRE_FALSE(b.hasValidPosition());
    }


    SECTION("Adding multiple pieces") {
        SECTION("Fill with same piece same color") {
            uint32_t size = 4;
            Board b = Board::emptyBoard(size);

            Piece piece = GENERATE_PIECE();

            // overshoot for index check
            for (uint32_t i = 0; i < size * size + 10; i++) {
                b.setPiece(i, piece);
            }

            CHECK(b.countPieces(piece.color()) == size * size);
            CHECK(b.countPieces(opposite(piece.color())) == 0);

            for (uint32_t i = 0; i < size * size + 10; i++) {
                CAPTURE(i);
                if (i < size * size) {
                    CHECK(b.pieceAt(i) == piece);
                }
            }
        }

        SECTION("Fill board with alternating colors") {
            uint32_t size = 4;
            Board b = Board::emptyBoard(size);

            Piece piece = GENERATE_PIECES_WHITE_ONLY();
            Piece pieceOther(piece.type(), opposite(piece.color()));

            // overshoot for index check
            for (uint32_t i = 0; i < size * size + 10; i++) {
                if (i % 2 == 0) {
                    b.setPiece(i, piece);
                } else {
                    b.setPiece(i, pieceOther);
                }
            }

            CHECK(b.countPieces(piece.color()) == size * size / 2);
            CHECK(b.countPieces(opposite(piece.color())) == size * size / 2);

            for (uint32_t i = 0; i < size * size + 10; i++) {
                CAPTURE(i);
                if (i < size * size) {
                    if (i % 2 == 0) {
                        CHECK(b.pieceAt(i) == piece);
                    } else {
                        CHECK(b.pieceAt(i) == pieceOther);
                    }
                }
            }
        }
    }


//    SECTION("Valid boards") { TODO
//        SECTION("Standard board is valid") {
//            auto board = Board::standardBoard();
//            REQUIRE(board.hasValidPosition());
//        }
//    }

}
