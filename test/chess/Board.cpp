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

        CHECK(b.size() == size);

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

        uint32_t index = GENERATE_COPY(0u, size - 1, size * size - 1, size * size, take(3, random(1u, std::max(1u, size * size - 1))));

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
            auto pieceHere = b.pieceAt(i);
            REQUIRE(pieceHere.has_value() == (i == index && index < size * size));
        }

        if (index < size * size) {
            REQUIRE(b.pieceAt(index) == piece);
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

        uint32_t index = GENERATE_COPY(0u, size - 1, size * size - 1, size * size, take(3, random(1u, std::max(1u, size * size - 1))));

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

        uint32_t index = GENERATE_COPY(0u, size - 1, size * size - 1, size * size, take(3, random(1u, std::max(1u, size * size - 1))));

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


TEST_CASE("Basic FEN parsing", "[chess][parsing][fen]") {
    using namespace Chess;
    SECTION("Wrong inputs") {
        auto fails = [](const std::string& s) {
            CAPTURE(s);
            ExpectedBoard b = Board::fromFEN(s);
            if (b) {
                INFO("Size " << b.value().size());
                INFO("White pieces: #" << b.value().countPieces(Color::White));
                INFO("Black pieces: #" << b.value().countPieces(Color::Black));

                CHECK_FALSE(b);
            } else {
                // should have some error
                CHECK_FALSE(b.error().empty());
            }

        };

        fails("");
        fails("bla bla");

        fails("8 no other things left here");
        fails("8/8/8/8/8/8/8/notapieceandtoolong w - - 0 1");
        fails("4/8/8/8/8/8/8/8 w - - 0 1");
        fails("8/8/8/8/8/8/8/4 w - - 0 1");
        fails("8/8/4/8/8/8/8/8 w - - 0 1");
        fails("44/8/8/8/8/8/8/8 w - - 0 1");
    }

    auto is_valid_board = [](ExpectedBoard& board) {
        if (board) {
            return;
        }
        INFO("Did not create valid board: " << board.error());
        REQUIRE(board);
    };

    SECTION("Empty FEN") {
        std::string color = GENERATE("w", "b");
        CAPTURE(color);
        ExpectedBoard b = Board::fromFEN("8/8/8/8/8/8/8/8 " + color + " - - 0 1");
        is_valid_board(b);

        Board board = b.extract();
        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == 0);
        CHECK(board.countPieces(Color::Black) == 0);
        CHECK_FALSE(board.hasValidPosition());
        CHECK(board.colorToMove() == (color == "w" ? Color::White : Color::Black));
    }

    SECTION("Adds pawn in FEN") {
        std::string pieceFEN = GENERATE("p", "P");
        CAPTURE(pieceFEN);
        bool isWhite = pieceFEN == "P";
        ExpectedBoard b = Board::fromFEN(pieceFEN + "7/8/8/8/8/8/8/8 w - - 0 1");
        is_valid_board(b);

        Board board = b.extract();
        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == (isWhite ? 1 : 0));
        CHECK(board.countPieces(Color::Black) == (isWhite ? 0 : 1));

        auto p =  board.pieceAt(0);
        REQUIRE(p);
        Piece& piece = *p;
        REQUIRE(piece.type() == Piece::Type::Pawn);
        REQUIRE(piece.color() == (isWhite ? Color::White : Color::Black));
    }

    SECTION("Can read multiple pieces consecutively") {
        std::string pieceFEN = GENERATE("pN", "Pn");
        CAPTURE(pieceFEN);
        ExpectedBoard b = Board::fromFEN(pieceFEN + "6/8/8/8/8/8/8/8 w - - 0 1");
        is_valid_board(b);

        Board board = b.extract();
        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == 1);
        CHECK(board.countPieces(Color::Black) == 1);

        auto p =  board.pieceAt(0);
        REQUIRE(p);
        Piece& piece1 = *p;

        auto p2 =  board.pieceAt(1);
        REQUIRE(p2);
        Piece& piece2 = *p2;

        CHECK(piece1.type() == Piece::Type::Pawn);
        CHECK(piece2.type() == Piece::Type::Knight);
        REQUIRE(piece1.color() != piece2.color());
    }

    SECTION("Can read full board of anypiece") {
        std::string pieceFEN = GENERATE("pppppppp", "nnnnnnnn", "kkkkkkkk", "qqqqqqqq", "rrrrrrrr", "bbbbbbbb");
        bool upper = GENERATE(true, false);
        if (upper) {
            std::transform(pieceFEN.begin(), pieceFEN.end(), pieceFEN.begin(), [](unsigned char c){ return toupper(c); });
        }
        CAPTURE(pieceFEN);

        auto ePiece = Piece::fromFEN(pieceFEN[0]);
        REQUIRE(ePiece);
        Piece expectedPiece = *ePiece;

        ExpectedBoard b = Board::fromFEN(pieceFEN + "/" + pieceFEN + "/" + pieceFEN + "/" + pieceFEN +
                                   "/" + pieceFEN + "/" + pieceFEN + "/" + pieceFEN + "/" + pieceFEN +
                                   " w - - 0 1");
        is_valid_board(b);

        Board board = b.extract();
        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == (upper ? 64 : 0));
        CHECK(board.countPieces(Color::Black) == (upper ? 0 : 64));

        for (uint32_t i = 0; i < 64; i++) {
            REQUIRE(board.pieceAt(i) == expectedPiece);
        }
    }

    SECTION("Can read all kinds of different pieces in the same board") {
        std::vector<Piece> expectedPieces = {
                Piece(Piece::Type::Pawn, Color::Black),
                Piece(Piece::Type::Knight, Color::Black),
                Piece(Piece::Type::King, Color::Black),
                Piece(Piece::Type::Queen, Color::Black),
                Piece(Piece::Type::Rook, Color::Black),
                Piece(Piece::Type::Bishop, Color::Black),
                Piece(Piece::Type::Pawn, Color::White),
                Piece(Piece::Type::King, Color::White),
        };

        ExpectedBoard b = Board::fromFEN("pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK w - - 0 1");
        is_valid_board(b);

        Board board = b.extract();
        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == 2 * 8);
        CHECK(board.countPieces(Color::Black) == 6 * 8);

        for (uint32_t i = 0; i < 64; i++) {
            REQUIRE(board.pieceAt(i) == expectedPieces[i % 8]);
        }
    }
}
