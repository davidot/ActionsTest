#include <catch2/catch.hpp>
#include <chess/Board.h>

using namespace Chess;

TEST_CASE("Board", "[chess][base]") {

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
            CHECK(b.countPieces(opposite(piece.color())) == 0);
        } else {
            CHECK(b.countPieces(piece.color()) == 1);
            CHECK(b.countPieces(opposite(piece.color())) == 0);
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
            CHECK(b.countPieces(opposite(piece.color())) == 0);
            CHECK_FALSE(b.pieceAt(index).has_value());
        }
    }

    SECTION("Can add pieces every where using column and row") {
        int32_t size = GENERATE(1, 8, 13, 103, 180, 255);
        Piece piece = GENERATE_PIECE();
        Board b = Board::emptyBoard(size);

        uint32_t column = GENERATE_COPY(0u, size - 1, take(4u, random(0u, uint32_t(std::max(1, size - 2)))));
        uint32_t row = GENERATE_COPY(0u, size - 1, take(4u, random(0u, uint32_t(std::max(1, size - 2)))));

        if (size == 1 && (row > 0 || column > 0)) {
            return;
        }

        CAPTURE(size, piece, column, row);

        b.setPiece(column, row, piece);

        CHECK(b.countPieces(piece.color()) == 1);
        CHECK(b.countPieces(opposite(piece.color())) == 0);
        REQUIRE(b.pieceAt(column, row).has_value());

        // since we only add once piece it can never be a valid position!
        CHECK_FALSE(b.hasValidPosition());

        for (uint8_t c = column - 3; c < uint8_t(column + 3); c++) {
            for (uint8_t r = row - 3; r < uint8_t(row + 3); r++) {
                REQUIRE(b.pieceAt(c, r).has_value() == (r == row && c == column));
            }
        }

        SECTION("Remove piece") {
            b.setPiece(column, row, std::nullopt);

            CHECK(b.countPieces(piece.color()) == 0);
            CHECK(b.countPieces(opposite(piece.color())) == 0);
            CHECK_FALSE(b.pieceAt(column, row).has_value());
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
    SECTION("Wrong inputs") {
        auto failsBase = [](const std::string& s) {
            CAPTURE(s);
            ExpectedBoard b = Board::fromFEN(s);
            if (b) {
                INFO("Size " << b.value().size());
                INFO("White pieces: #" << b.value().countPieces(Color::White));
                INFO("Black pieces: #" << b.value().countPieces(Color::Black));

                CHECK_FALSE(b);
            } else {
                // should have some error
                REQUIRE_FALSE(b.error().empty());
//                WARN(s << " got error: " << b.error());
            }

        };

        SECTION("Completely invalid formats") {
            failsBase("");
            failsBase("bla bla");
            failsBase("8 no other things left here");
        }


        SECTION("Board layout") {
            auto fails = [&failsBase](const std::string& s) {
                failsBase(s + " w - - 0 1");
            };

            fails("8p");
            fails("p");
            fails("p/p/p/p/p/p/p/p");

            fails("8/8/8/8/8/8/8/notapieceandtoolong");
            fails("8/8/8/notapieceandtoolong/8/8/8/8");
            fails("8/notapieceandtoolong/8/8/8/8/8/8");
            fails("notapieceandtoolong/8/8/8/8/8/8/8");

            fails("9/9/9/9/9/9/9/9");

            fails("8/8/8/8/8/8/8/8/1");
            fails("8/8/8/8/8/8/8/1/7");
            fails("8/8/8/8/8/8/8/8/2");
            fails("8/8/8/8/8/8/8/8/8");
            fails("1/8/8/8/8/8/8/8/8");

            fails("4/8/8/8/8/8/8/8");
            fails("8/8/8/8/8/8/8/4");
            fails("8/8/4/8/8/8/8/8");

            fails("44/8/8/8/8/8/8/8");
            fails("45/8/8/8/8/8/8/8");
            fails("8/8/8/8/8/8/8/44");

            fails("p8/8/8/8/8/8/8/8");
            fails("p7p/8/8/8/8/8/8/8");
            fails("7pp/8/8/8/8/8/8/8");

            fails("pp7/8/8/8/8/8/8/8");
            fails("8/8/8/8/8/8/8/p8");
        }

        SECTION("Metadata") {
            auto fails = [&failsBase](const std::string& s) {
              failsBase("8/8/8/8/8/8/8/8 " + s);
            };

            fails("x - - 0 1");
            fails("y - - 0 1");
            fails("white - - 0 1");
            fails("black - - 0 1");
            fails("imnotsurewhostartonchess?? - - 0 1");
            fails("? - - 0 1");
            fails("ww - - 0 1");
            fails("bw - - 0 1");
            fails("wb - - 0 1");
            fails("bb - - 0 1");

            fails("w x - 0 1");

            fails("w - x 0 1");
            fails("w - a9 0 1");
            fails("w - h-1 0 1");
            fails("w - h0 0 1");
            fails("w - a- 0 1");
            fails("w - -a 0 1");

            fails("w - - x 1");
            fails("w - - -1 1");
            fails("w - - 0.3 1");

            fails("w - - 0 x");
            fails("w - - 0 -1");
            fails("w - - 0 0.3");
        }

    }

    auto is_valid_board = [](ExpectedBoard& board) {
        if (board) {
            return;
        }
        INFO("Did not create valid board: " << board.error());
        REQUIRE(board);
    };

    SECTION("Empty FEN") {
        ExpectedBoard b = Board::fromFEN("8/8/8/8/8/8/8/8 w - - 0 1");
        is_valid_board(b);

        Board board = b.extract();
        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == 0);
        CHECK(board.countPieces(Color::Black) == 0);
        CHECK_FALSE(board.hasValidPosition());
    }

    SECTION("Reads color of next turn") {
        std::string color = GENERATE("w", "b");
        CAPTURE(color);
        ExpectedBoard b = Board::fromFEN("8/8/8/8/8/8/8/8 " + color + " - - 0 1");
        is_valid_board(b);

        Board board = b.extract();
        REQUIRE(board.size() == 8);
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

        auto p =  board.pieceAt(0, 7);
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

        auto p =  board.pieceAt(0, 7);
        REQUIRE(p);
        Piece& piece1 = *p;

        auto p2 =  board.pieceAt(1, 7);
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
        // Note this actually makes assumptions about the index order
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

        for (uint32_t i = 0; i < 8; i++) {
            for (uint32_t j = 0; j < 8; j++) {
                REQUIRE(board.pieceAt(j, i) == expectedPieces[j]);
            }
        }
    }
}

TEST_CASE("Basic SAN parsing", "[chess][parsing][san]") {

    Piece filledPiece = Piece::fromFEN('p').value(); // black pawn
    Piece otherPiece = Piece(Chess::Piece::Type::Queen, Color::White); // white queen
    REQUIRE_FALSE(filledPiece == otherPiece);

    auto b = Board::fromFEN("pppppppp/pppppppp/pppppppp/pppppppp/pppppppp/pppppppp/pppppppp/pppppppp w - - 0 1");
    REQUIRE(b);
    Board filledBoard = b.extract();

    SECTION("Correct squares") {
        auto is_position = [&](uint8_t row, uint8_t column, const std::string& san) {
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
        };

        is_position(0, 0, "a1");
        is_position(0, 1, "b1");
        is_position(1, 0, "a2");
        is_position(1, 2, "c2");
        is_position(3, 3, "d4");
        is_position(7, 7, "h8");
        is_position(0, 7, "h1");
    }

    SECTION("Failing squares") {
        Board empty = Board::emptyBoard();

        auto is_not_a_position = [&](const std::string& san) {
            CAPTURE(san);
            REQUIRE_FALSE(filledBoard.pieceAt(san).has_value());
            empty.setPiece(san, Piece(Piece::Type::Pawn, Color::Black));
            REQUIRE(empty.countPieces(Color::Black) == 0);
            REQUIRE(empty.countPieces(Color::White) == 0);
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
