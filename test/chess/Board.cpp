#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/Board.h>

using namespace Chess;

TEST_CASE("Board", "[chess][base]") {

#define GENERATE_PIECE() Piece(GENERATE(TEST_SOME(values({Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King}))), GENERATE(Color::White, Color::Black))
#define GENERATE_PIECES_WHITE_ONLY() Piece(GENERATE(TEST_SOME(values({Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King}))), Color::White)

    SECTION("Empty board has no pieces") {
        Board b = Board::emptyBoard();

        REQUIRE(b.size() == 8);

        REQUIRE(b.countPieces(Color::White) == 0);
        REQUIRE(b.countPieces(Color::Black) == 0);
        REQUIRE_FALSE(b.hasValidPosition());

        for (uint8_t i = 0; i < 8; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                REQUIRE_FALSE(b.pieceAt(i, j));
            }
        }

        CHECK(b.colorToMove() == Color::White);
        b.makeNullMove();
        CHECK(b.colorToMove() == Color::Black);
        b.undoNullMove();
        CHECK(b.colorToMove() == Color::White);
    }

    SECTION("Can add pieces every where") {
        Piece piece = GENERATE_PIECE();
        Board b = Board::emptyBoard();

        uint8_t size = 8;
        uint8_t col = GENERATE(TEST_SOME(range(0, 10)));
        uint8_t row = GENERATE(TEST_SOME(range(0, 10)));

        CAPTURE(piece, col, row);

        b.setPiece(col, row, piece);

        if (col >= size || row >= size) {
            // out of bounds
            CHECK(b.countPieces(piece.color()) == 0);
            CHECK(b.countPieces(opposite(piece.color())) == 0);
        } else {
            CHECK(b.countPieces(piece.color()) == 1);
            CHECK(b.countPieces(opposite(piece.color())) == 0);
        }

        // since we only add once piece it can never be a valid position!
        CHECK_FALSE(b.hasValidPosition());

        for (uint8_t i = 0; i < 8; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                if (i == col && j == row) {
                    REQUIRE(b.pieceAt(i, j) == piece);
                } else {
                    REQUIRE_FALSE(b.pieceAt(i, j));
                }

            }
        }

        b.setPiece(col, row, std::nullopt);

        CHECK(b.countPieces(piece.color()) == 0);
        CHECK(b.countPieces(opposite(piece.color())) == 0);
        CHECK_FALSE(b.pieceAt(col, row).has_value());
    }

    SECTION("Can add pieces every where using column and row") {
        Piece piece = GENERATE_PIECE();
        Board b = Board::emptyBoard();

        int32_t size = 8;
        uint32_t column = GENERATE_COPY(0u, size - 1, take(1u, random(0u, uint32_t(std::max(1, size - 2)))));
        uint32_t row = GENERATE_COPY(0u, size - 1, take(1u, random(0u, uint32_t(std::max(1, size - 2)))));

        CAPTURE(piece, column, row);

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
        Piece piece = GENERATE_PIECE();
        Board b = Board::emptyBoard();

        uint32_t size = 8;
        uint8_t col = GENERATE(TEST_SOME(range(0, 10)));
        uint8_t row = GENERATE(TEST_SOME(range(0, 10)));

        CAPTURE(size, piece, col, row);

        b.setPiece(col, row, piece);

        b.setPiece(col, row, piece);

        if (col >= size || row >= size) {
            // out of bounds
            REQUIRE(b.countPieces(piece.color()) == 0);
        } else {
            REQUIRE(b.countPieces(piece.color()) == 1);
        }

        // since we only add once piece it can never be a valid position!
        REQUIRE_FALSE(b.hasValidPosition());

    }

    SECTION("Remove non-existent piece") {
        Board b = Board::emptyBoard();

        uint32_t size = 8;
        uint8_t col = GENERATE(TEST_SOME(range(0, 10)));
        uint8_t row = GENERATE(TEST_SOME(range(0, 10)));

        CAPTURE(size, col, row);


        // double set no piece (and should already be no piece of course
        b.setPiece(col, row, std::nullopt);

        b.setPiece(col, row, std::nullopt);

        REQUIRE(b.countPieces(Color::White) == 0);
        REQUIRE(b.countPieces(Color::Black) == 0);

        // since we only add once piece it can never be a valid position!
        REQUIRE_FALSE(b.hasValidPosition());
    }


    SECTION("Adding multiple pieces") {
        SECTION("Fill with same piece same color") {
            uint32_t size = 8;
            Board b = Board::emptyBoard();

            Piece piece = GENERATE_PIECE();
            // overshoot for index check
            for (uint8_t i = 0; i < 10; i++) {
                for (uint8_t j = 0; j < 10; j++) {
                    b.setPiece(i, j, piece);
                }
            }

            REQUIRE(b.countPieces(piece.color()) == size * size);
            REQUIRE(b.countPieces(opposite(piece.color())) == 0);

            for (uint8_t i = 0; i < 10; i++) {
                for (uint8_t j = 0; j < 10; j++) {
                    if (i < size && j < size) {
                        REQUIRE(b.pieceAt(i, j) == piece);
                    }
                }
            }
        }

        SECTION("Fill board with alternating colors") {
            uint32_t size = 8;
            Board b = Board::emptyBoard();

            Piece piece = GENERATE_PIECES_WHITE_ONLY();
            Piece pieceOther(piece.type(), opposite(piece.color()));

            // overshoot for index check
            for (uint8_t col = 0; col < 10; col++) {
                for (uint8_t row = 0; row < 10; row++) {
                    if ((col + row) % 2 == 0) {
                        b.setPiece(col, row, piece);
                    } else {
                        b.setPiece(col, row, pieceOther);
                    }
                }
            }

            REQUIRE(b.countPieces(piece.color()) == size * size / 2);
            REQUIRE(b.countPieces(opposite(piece.color())) == size * size / 2);

            for (uint8_t i = 0; i < 10; i++) {
                for (uint8_t j = 0; j < 10; j++) {
                    if (i < size && j < size) {
                        if ((i + j) % 2 == 0) {
                            REQUIRE(b.pieceAt(i, j) == piece);
                        } else {
                            REQUIRE(b.pieceAt(i, j) == pieceOther);
                        }
                    } else {
                        REQUIRE_FALSE(b.pieceAt(i, j));
                    }
                }
            }

        }
    }

}

TEST_CASE("Validity of boards", "[.][chess][rules][board]") {
    // TODO
    SECTION("Empty board is invalid") {
        auto board = Board::emptyBoard();
        REQUIRE_FALSE(board.hasValidPosition());
    }

    SECTION("Standard board is valid") {
        auto board = Board::standardBoard();
        REQUIRE(board.hasValidPosition());
    }
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
            failsBase("\n");
            failsBase("\r");
            failsBase("\b");
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

            fails("8/8/8/8/8/8/8/8/");

            fails("8/8/8/8/8/8/8/8/1");
            fails("8/8/8/8/8/8/8/1/7");
            fails("8/8/8/8/8/8/8/8/2");
            fails("8/8/8/8/8/8/8/8/8");
            fails("1/8/8/8/8/8/8/8/8");

            fails("4/8/8/8/8/8/8/8");
            fails("8/8/8/8/8/8/8/4");
            fails("8/8/4/8/8/8/8/8");

            fails("8/8/8/8/8/8/8/ppppppppp");
            fails("8/8/8/8/8/8/8/6ppp");
            fails("8/8/8/8/8/8/8/7pp");
            fails("8/8/8/8/8/8/8/8p");

            fails("44/8/8/8/8/8/8/8");
            fails("45/8/8/8/8/8/8/8");
            fails("8/8/8/8/8/8/8/44");
            fails("8/8/8/8/8/8/8/404");
            fails("8/8/8/8/8/8/8/08");
            fails("800/8/8/8/8/8/8/08");

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
            fails("w xxxxx - 0 1");
            fails("w qqqqq - 0 1");
            fails("w KqQQk - 0 1");
            fails("w KKQQ - 0 1");
            fails("w kqK - 0 1");
            fails("w kqKQ - 0 1");
            fails("w kKQ - 0 1");
            fails("w qKQ - 0 1");
            fails("w qQ - 0 1");
            fails("w kK - 0 1");
            fails("w qK - 0 1");
            fails("w KQkQ - 0 1");
            fails("w QkKq - 0 1");
            fails("w kqQ - 0 1");
            fails("w pPpP - 0 1");
            fails("w RBrb - 0 1");

            fails("w - x 0 1");
            fails("w - a9 0 1");
            fails("w - h-1 0 1");
            fails("w - h0 0 1");
            fails("w - h9 0 1");
            fails("w - a9 0 1");
            fails("w - g9 0 1");
            fails("w - i6 0 1");
            fails("w - a- 0 1");
            fails("w - -a 0 1");

            fails("w - - x 1");
            fails("w - - -1 1");
            fails("w - - 0.3 1");
            fails("w - - 0f 1");
            fails("w - - 1f 1");

            fails("w - - 0 x");
            fails("w - - 0 -1");
            fails("w - - 0 0.3");
            fails("w - - 0 0a");
            fails("w - - 0 1a");
        }

        SECTION("Missing parts") {
            failsBase("  w - - 1 0");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR  - - 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w  - 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w -  0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - -  1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 ");
        }

        SECTION("Invalid en passant moves") {
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - a1 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - h1 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - h8 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - h5 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - a7 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - c2 0 1");
            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - a4 0 1");
            // FIXME: these moves are really not valid and it should fail!
//            failsBase("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - a3 0 1"); Not valid due to no pawn
//            failsBase("rnbqkbnr/pppppppp/8/8/PPPPPPPP/PPPPPPPP/PPPPPPPP/RNBQKBNR w - a3 0 1"); Not valid due to non empty square
        }

    }

    auto is_valid_board = [](const std::string& str) {
        ExpectedBoard board = Board::fromFEN(str);
        if (board) {
            REQUIRE(board.value().toFEN() == str);

            return std::move(board.extract());
        }
        INFO("Did not create valid board: " << board.error());
        REQUIRE(board);

        return Board::emptyBoard();
    };

    SECTION("Empty FEN") {
        Board board = is_valid_board("8/8/8/8/8/8/8/8 w - - 0 1");
        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == 0);
        CHECK(board.countPieces(Color::Black) == 0);
        CHECK_FALSE(board.hasValidPosition());
    }

    SECTION("Reads color of next turn") {
        std::string color = GENERATE("w", "b");
        CAPTURE(color);
        Board board = is_valid_board("8/8/8/8/8/8/8/8 " + color + " - - 0 1");

        REQUIRE(board.size() == 8);
        CHECK(board.colorToMove() == (color == "w" ? Color::White : Color::Black));
    }

    SECTION("Adds pawn in FEN") {
        std::string pieceFEN = GENERATE("p", "P");
        CAPTURE(pieceFEN);
        bool isWhite = pieceFEN == "P";
        Board board = is_valid_board(pieceFEN + "7/8/8/8/8/8/8/8 w - - 0 1");

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
        Board board = is_valid_board(pieceFEN + "6/8/8/8/8/8/8/8 w - - 0 1");

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

        Board board = is_valid_board(pieceFEN + "/" + pieceFEN + "/" + pieceFEN + "/" + pieceFEN +
                                     "/" + pieceFEN + "/" + pieceFEN + "/" + pieceFEN + "/" + pieceFEN +
                                     " w - - 0 1");

        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == (upper ? 64 : 0));
        CHECK(board.countPieces(Color::Black) == (upper ? 0 : 64));

        for (uint8_t i = 0; i < 8; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                REQUIRE(board.pieceAt(i, j) == expectedPiece);
            }
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
        Board board = is_valid_board("pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK/pnkqrbPK w - - 0 1");

        REQUIRE(board.size() == 8);
        CHECK(board.countPieces(Color::White) == 2 * 8);
        CHECK(board.countPieces(Color::Black) == 6 * 8);

        for (uint32_t i = 0; i < 8; i++) {
            for (uint32_t j = 0; j < 8; j++) {
                REQUIRE(board.pieceAt(j, i) == expectedPieces[j]);
            }
        }
    }

    SECTION("Side to move state") {
        std::string col = GENERATE("w", "b");
        CAPTURE(col);
        std::string basePosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR " + col + " KQkq - 0 1";
        auto board = is_valid_board(basePosition);
        if (col == "w") {
            REQUIRE(board.colorToMove() == Color::White);
        } else {
            REQUIRE(board.colorToMove() == Color::Black);
        }
    }

    SECTION("Castling state") {
        std::string castling = GENERATE("-", "KQkq", "KQ", "kq", "Kk", "Qq", "Kq", "Qk", "K", "Q", "k", "q");
        CAPTURE(castling);
        std::string basePosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w " + castling + " - 0 1";
        auto b = is_valid_board(basePosition);
        // TODO: check we can actually make the appropriate castling moves
        if (castling == "-") {
            REQUIRE(b.castlingRights() == CastlingRight::NO_CASTLING);
        }
        // TODO: how to check this without actually replicating the logic??
        REQUIRE("TODO");
    }

    SECTION("En passant state") {
        std::string enPassant = GENERATE(as<std::string>(), "a", "b", "h") + GENERATE("3", "6");
        CAPTURE(enPassant);
        std::string basePosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - " + enPassant + " 0 1";
        // FIXME: these moves are really not valid and it should fail!
        is_valid_board(basePosition);
        // TODO verify somehow?
        REQUIRE("TODO");
    }

    SECTION("Halfmoves since capture or pawn move") {
        uint32_t moves = GENERATE(0u, 1u, 25u, 50u, 100u);
        CAPTURE(moves);
        std::string basePosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - " + std::to_string(moves) + " 1";
        is_valid_board(basePosition);
        // TODO verify somehow?
        REQUIRE("TODO");
    }

    SECTION("Full move number") {
        uint32_t moves = GENERATE(0u, 1u, 25u, 50u, 100u, 128u, 3000u, 8849u);
        CAPTURE(moves);
        std::string basePosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 " + std::to_string(moves);
        is_valid_board(basePosition);
        // TODO verify somehow?
        REQUIRE("TODO");
    }

    SECTION("Example positions") {
        SECTION("Start position") {
            std::string position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            Board board = is_valid_board(position);

            REQUIRE(board.toFEN() == position);

            Board standard = Board::standardBoard();
            REQUIRE(board.size() == standard.size());
            REQUIRE(board.countPieces(Color::White) == standard.countPieces(Color::White));
            REQUIRE(board.countPieces(Color::Black) == standard.countPieces(Color::Black));
            // ehh not sure we want this
            REQUIRE(board.countPieces(Color::White) == standard.countPieces(Color::Black));

            for (uint32_t i = 0; i < 8; i++) {
                for (uint32_t j = 0; j < 8; j++) {
                    REQUIRE(board.pieceAt(j, i) == standard.pieceAt(j, i));
                }
            }
        }

        SECTION("Empty board") {
            Board empty = Board::emptyBoard();
            REQUIRE(empty.size() == 8);

            std::string position = "8/8/8/8/8/8/8/8 w - - 0 1";
            REQUIRE(empty.toFEN() == position);

            Board board = is_valid_board(position);
            REQUIRE(board.toFEN() == position);

            REQUIRE(board.size() == empty.size());
            REQUIRE(board.countPieces(Color::White) == empty.countPieces(Color::White));
            REQUIRE(board.countPieces(Color::Black) == empty.countPieces(Color::Black));

            for (uint32_t i = 0; i < 8; i++) {
                for (uint32_t j = 0; j < 8; j++) {
                    REQUIRE(board.pieceAt(j, i) == empty.pieceAt(j, i));
                }
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
        auto is_position = [&](uint8_t column, uint8_t row, const std::string& san) {
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

        auto is_not_a_position = [&](const std::string& san) {
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


TEST_CASE("Basic FEN output", "[chess][parsing][fen]") {
    // This assumes correct SAN parsing
    SECTION("Empty board generates empty FEN") {
        Board board = Board::emptyBoard();
        REQUIRE(board.toFEN() == "8/8/8/8/8/8/8/8 w - - 0 1");
    }

    SECTION("If piece set it is shown in FEN") {
        Board board = Board::emptyBoard();
        Piece p = GENERATE_PIECE();
        board.setPiece("a8", p);
        auto fen = board.toFEN();
        REQUIRE(fen[0] == p.toFEN());
    }

    SECTION("Filling the board gives filled FEN") {
        Board board = Board::emptyBoard();
        Piece p = GENERATE_PIECE();
        for (uint8_t col = 0; col < 8; col++) {
            for (uint8_t row = 0; row < 8; row++) {
                board.setPiece(col, row, p);
            }
        }
        const auto fen = board.toFEN();
        auto it = fen.cbegin();
        int i = 0;
        char pFEN = p.toFEN();
        CAPTURE(fen, pFEN);
        while (it != fen.cend() && *it != ' ') {
            CAPTURE(i);
            if (i == 8) {
                CHECK(*it == '/');
                i = 0;
            } else {
                CHECK(*it == pFEN);
                i++;
            }
            ++it;
        }
        REQUIRE(it != fen.cend());
    }

    SECTION("Single piece in some row first/last col") {
        uint8_t row = GENERATE(TEST_SOME(range(0u, 8u)));
        Piece p = GENERATE_PIECE();
        char pFEN = p.toFEN();

        bool first = GENERATE(true, false);

        Board board = Board::emptyBoard();
        board.setPiece(first ? 0 : 7, row, p);

        const auto fen = board.toFEN();
        CAPTURE(fen, pFEN, row);

        char firstChar = first ? pFEN : '7';
        char secondChar = first ? '7' : pFEN;

        auto it = fen.cbegin();
        int currRow = 7;
        while (it != fen.cend()) {
            CAPTURE(currRow);
            REQUIRE(currRow >= 0);
            if (currRow == row) {
                REQUIRE(*it == firstChar);
                ++it;
                REQUIRE_FALSE(it == fen.cend());
                REQUIRE(*it == secondChar);
                ++it;
            } else {
                REQUIRE(*it == '8');
                ++it;
            }
            currRow--;
            if (it == fen.cend() || *it == ' ') {
                break;
            }
            REQUIRE(*it == '/');
            ++it;
        }
        REQUIRE(it != fen.cend());
    }

    SECTION("From and to (valid) FEN is identical") {
        auto checkIdentical = [](const std::string& str) {
            CAPTURE(str);
            auto expB = Board::fromFEN(str);
            if (!expB) {
                WARN("Got invalid board: " << expB.error());
                REQUIRE(expB);
                return;
            }
            auto board = expB.extract();
            REQUIRE(board.toFEN() == str);
        };

        checkIdentical("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        checkIdentical("8/8/8/8/8/8/8/8 w - - 0 1");
        SECTION("Single piece") {
            std::string pawn = GENERATE("p", "P", "q", "Q", "k", "K", "r", "b", "n");
            checkIdentical("3" + pawn + "4/8/8/8/8/8/8/8 w - - 0 1");
            checkIdentical("4" + pawn + "3/8/8/8/8/8/8/8 w - - 0 1");
            checkIdentical("8/4" + pawn + "3/8/8/8/8/8/8 w - - 0 1");
            checkIdentical("8/3" + pawn + "4/8/8/8/8/8/8 w - - 0 1");
        }

    }
}
