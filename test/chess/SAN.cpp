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
        REQUIRE(board.SANToColRow(col + std::to_string(row + 1)).has_value());
    }
}


TEST_CASE("SAN move parsing", "[chess][parsing][san][move]") {

    //  file of departure if different
    //  rank of departure if the files are the same but the ranks differ
    //  the complete origin square coordinate otherwise

    // en passant hits on destination square
    // only ambiguity on valid moves!
    // example of pinned piece forcing PGN non ambiguous (Q in row 1)
    // 2k5/3p4/4Q3/8/4q1p1/8/4Q3/4K3 w - - 0 1

#define CHECK_MOVE_WITH_NAME(mv, nm)                               \
    do {                                                           \
        Move expectedMove = (mv);                                  \
        const std::string& expectedName = (nm);                    \
        auto parsedMove = board.parseSANMove(nm);                  \
        auto moveName = board.moveToSAN(expectedMove);             \
        CAPTURE(expectedMove, expectedName, parsedMove, moveName); \
        CHECK(parsedMove == expectedMove);                         \
        CHECK(moveName == expectedName);                           \
        REQUIRE(parsedMove.has_value());                           \
    } while (false)


    SECTION("Castling") {
        Color color = GENERATE(Color::White, Color::Black);
        Board board = TestUtil::generateCastlingBoard(color, true, true, false, true);

        bool kingSide = GENERATE(true, false);
        std::string castleSAN = "O-O";
        if (!kingSide) {
            castleSAN += "-O";
        }
        CAPTURE(color, kingSide);
        BoardIndex home = Board::homeRow(color);
        Move move{Board::kingCol, home, kingSide ? Board::kingSideRookCol : Board::queenSideRookCol, home, Move::Flag::Castling};

        CHECK_MOVE_WITH_NAME(move, castleSAN);
    };

    SECTION("Non-pawn moves") {
        Board board = Board::emptyBoard();
        Color color = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != color) {
            board.makeNullMove();
        }

#define BASIC_MOVE_CHECK(fromCol, fromRow, toCol, toRow, pieceStr) \
    CHECK_MOVE_WITH_NAME(Move(fromCol, fromRow, toCol, toRow), pieceStr + Board::columnRowToSAN(toCol, toRow)); \
    board.setPiece(toCol, toRow, Piece{Piece::Type::Bishop, opposite(color)}); \
    CHECK_MOVE_WITH_NAME(Move(fromCol, fromRow, toCol, toRow), pieceStr + ("x" + Board::columnRowToSAN(toCol, toRow)))

        SECTION("King move") {
            BoardIndex col = GENERATE(TEST_SOME(range(3, 5)));
            BoardIndex row = GENERATE_COPY(TEST_SOME(filter([=](uint8_t r) { return r != 4 || col != 4; }, range(3, 5))));

            board.setPiece(4, 4, Piece{Piece::Type::King, color});

            BASIC_MOVE_CHECK(4, 4, col, row, "K");
        }

        SECTION("Rook move") {
            board.setPiece(4, 4, Piece{Piece::Type::Rook, color});

            BoardIndex toCol = 4;
            BoardIndex toRow = 2;

            BASIC_MOVE_CHECK(4, 4, toCol, toRow, "R");
        }

        SECTION("Bishop move") {
            board.setPiece(4, 4, Piece{Piece::Type::Bishop, color});

            BoardIndex toCoord = GENERATE(TEST_SOME(filter([](int i) {return i != 4; }, range(0, 8))));

            BASIC_MOVE_CHECK(4, 4, toCoord, toCoord, "B");
        }

        SECTION("Knight move") {
            board.setPiece(4, 4, Piece{Piece::Type::Knight, color});

            BoardIndex toCol = 3;
            BoardIndex toRow = 2;

            BASIC_MOVE_CHECK(4, 4, toCol, toRow, "N");
        }

        SECTION("Queen move") {
            BoardIndex index = GENERATE(take(3, random(0u, 63u)));
            uint8_t fromCol = index % 8u;
            uint8_t fromRow = index / 8u;

            uint8_t toCol = GENERATE(TEST_SOME(range(0, 8)));
            CAPTURE(fromCol, fromRow, toCol);
            // complicated expression to only generate valid queen moves
            uint8_t toRow = GENERATE_COPY(
                    filter([=](uint8_t r) {
                             return (fromCol != toCol || fromRow != r) &&
                                    (fromCol == toCol || fromRow == r ||
                                     (fromCol - fromRow) == (toCol - r) ||
                                     ((7 - fromRow) - fromCol) == ((7 - r) - toCol));
                           },
                           range(0, 8)));

            const Piece piece = Piece{Piece::Type::Queen, color};
            board.setPiece(fromCol, fromRow, piece);

            BASIC_MOVE_CHECK(fromCol, fromRow, toCol, toRow, "Q");
        }

#undef BASIC_MOVE_CHECK
    }

    SECTION("Ambiguity") {
        // 4 bishops can take rook
        // one needs full disam one just col one just row
        // B4k2/5B2/8/3r4/8/1B3B2/8/4K3 w - - 0 1
    }

    SECTION("Pawns") {
        SECTION("Can parse simple pawn move") {
            Board board = Board::standardBoard();

            std::string col = GENERATE(STRING_COLS);
            std::string posTo = col + "3";

            CHECK_MOVE_WITH_NAME(Move(col + "2", posTo), posTo);
        }

        SECTION("Can parse double push pawn move") {
            Board board = Board::standardBoard();

            std::string col = GENERATE(STRING_COLS);
            std::string posTo = col + "4";

            CHECK_MOVE_WITH_NAME(Move(col + "2", posTo, Move::Flag::DoublePushPawn), posTo);
        }

        SECTION("Can parse pawn promotion") {
            Board board = Board::emptyBoard();

            Color c = GENERATE(Color::White, Color::Black);
            if (board.colorToMove() != c) {
                board.makeNullMove();
            }

            BoardIndex row = Board::pawnHomeRow(opposite(c));
            BoardIndex promotionRow = Board::pawnPromotionRow(c);
            BoardIndex col = GENERATE(TEST_SOME(range(0, 8)));

            board.setPiece(col, row, Piece{Piece::Type::Pawn, c});

            std::string destSquare = Board::columnRowToSAN(col, promotionRow);

            Move::Flag flag = GENERATE(TEST_SOME(values({Move::Flag::PromotionToQueen, Move::Flag::PromotionToKnight,
                                                         Move::Flag::PromotionToBishop, Move::Flag::PromotionToRook})));

            Move mv{col, row, col, promotionRow, flag};

            REQUIRE(mv.isPromotion());
            std::string promoType = "x";
            promoType[0] = Piece{mv.promotedType(), Color::White}.toFEN();

            CHECK_MOVE_WITH_NAME(mv, destSquare + "=" + promoType);
        }

        SECTION("Can parse pawn capture") {
            Board board = Board::emptyBoard();

            Color color = GENERATE(Color::White, Color::Black);
            if (board.colorToMove() != color) {
                board.makeNullMove();
            }

            BoardIndex col = GENERATE(TEST_SOME(range(0, 8)));

            BoardIndex captureCol = col +
                               GENERATE_COPY(filter([col](int8_t i) {
                                 return col + i >= 0 && col + i < 8;
                               }, values({1, -1})));
            BoardIndex rowFrom = 4;
            BoardIndex rowTo = rowFrom + Board::pawnDirection(color);
            board.setPiece(col, rowFrom, Piece{Piece::Type::Pawn, color});
            board.setPiece(captureCol, rowTo, Piece{Piece::Type::Pawn, opposite(color)});


            std::string destSquare = Board::columnRowToSAN(captureCol, rowTo);
            std::string fromCol = Board::columnRowToSAN(col, rowTo).substr(0, 1);


            Move mv{col, rowFrom, captureCol, rowTo};
            CHECK_MOVE_WITH_NAME(mv, fromCol + "x" + destSquare);
        };

        SECTION("Can parse pawn promotion capture") {
            Board board = Board::emptyBoard();

            Color color = GENERATE(Color::White, Color::Black);
            if (board.colorToMove() != color) {
                board.makeNullMove();
            }

            BoardIndex col = GENERATE(TEST_SOME(range(0, 8)));

            BoardIndex captureCol = col +
                                    GENERATE_COPY(filter([col](int8_t i) {
                                      return col + i >= 0 && col + i < 8;
                                    }, values({1, -1})));

            BoardIndex rowFrom = Board::pawnHomeRow(opposite(color));
            BoardIndex rowTo = Board::pawnPromotionRow(color);
            board.setPiece(col, rowFrom, Piece{Piece::Type::Pawn, color});
            board.setPiece(captureCol, rowTo, Piece{Piece::Type::Pawn, opposite(color)});



            Move::Flag flag = GENERATE(TEST_SOME(values({Move::Flag::PromotionToQueen, Move::Flag::PromotionToKnight,
                                                         Move::Flag::PromotionToBishop, Move::Flag::PromotionToRook})));

            Move mv{col, rowFrom, captureCol, rowTo, flag};
            REQUIRE(mv.isPromotion());

            std::string destSquare = Board::columnRowToSAN(captureCol, rowTo);
            std::string fromCol = Board::columnRowToSAN(col, rowTo).substr(0, 1);

            CHECK_MOVE_WITH_NAME(mv, fromCol + "x" + destSquare + "=" + Piece(mv.promotedType(), Color::White).toFEN());
        }

        SECTION("Take enpassant") {
        }

        SECTION("Multiple pawns on the same column") {
            BoardIndex col = GENERATE(TEST_SOME(range(0, 8)));
            std::string colLetter = Board::columnRowToSAN(col, 0).substr(0, 1);
            Board board = Board::emptyBoard();

            Color color = GENERATE(Color::White, Color::Black);
            if (board.colorToMove() != color) {
                board.makeNullMove();
            }

            Piece pawn{Piece::Type::Pawn, color};

            SECTION("Handles push") {
                BoardIndex row1 = GENERATE(TEST_SOME(range(3, 5)));
                BoardIndex row2 = row1 - Board::pawnDirection(color);

                board.setPiece(col, row1, pawn);
                board.setPiece(col, row2, pawn);
                BoardIndex toRow = row1 + Board::pawnDirection(color);

                Move mv{col, row1, col, toRow};
                CHECK_MOVE_WITH_NAME(mv, Board::columnRowToSAN(col, toRow));
            };

            SECTION("Handles double push") {
                BoardIndex row1 = Board::pawnHomeRow(color);
                BoardIndex row2 = row1 + 3 * Board::pawnDirection(color);

                board.setPiece(col, row1, pawn);
                board.setPiece(col, row2, pawn);
                BoardIndex toRow = row1 + 2 * Board::pawnDirection(color);

                Move mv{col, row1, col, toRow, Move::Flag::DoublePushPawn};
                CHECK_MOVE_WITH_NAME(mv, Board::columnRowToSAN(col, toRow));
            }

            SECTION("Handles capture") {
                BoardIndex row1 = GENERATE(TEST_SOME(range(3, 5)));
                BoardIndex row2 = row1 - Board::pawnDirection(color);

                board.setPiece(col, row1, pawn);
                board.setPiece(col, row2, pawn);
                BoardIndex toRow = row1 + Board::pawnDirection(color);

                BoardIndex captureCol = col +
                                        GENERATE_COPY(filter([col](int8_t i) {
                                          return col + i >= 0 && col + i < 8;
                                        }, values({1, -1})));

                board.setPiece(captureCol, toRow, Piece{Piece::Type::Knight, opposite(color)});

                Move mv{col, row1, captureCol, toRow};
                CHECK_MOVE_WITH_NAME(mv, colLetter + "x" +Board::columnRowToSAN(captureCol, toRow));
            }
        }
    }

}
