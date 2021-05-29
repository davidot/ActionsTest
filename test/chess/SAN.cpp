#include "TestUtil.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
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
        auto parsedMove = board.parseSANMove(expectedName);                  \
        auto moveName = board.moveToSAN(expectedMove);             \
        CAPTURE(expectedMove, expectedName, parsedMove, moveName); \
        REQUIRE((parsedMove.has_value() && parsedMove == expectedMove && moveName == expectedName));                           \
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

        SECTION("Capture a piece") {
            board.setPiece(4, 4, Piece{Piece::Type::Knight, color});

            BoardIndex toCol = 5;
            BoardIndex toRow = 2;

            board.setPiece(toCol, toRow, Piece{Piece::Type::Knight, opposite(color)});

            CHECK_MOVE_WITH_NAME(Move(4, 4, toCol, toRow), "Nxf3");
        }

    }

    SECTION("Ambiguity") {

        Board board = Board::emptyBoard();
        Color color = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != color) {
            board.makeNullMove();
        }

        Piece bishop{Piece::Type::Bishop, color};
        Piece rook{Piece::Type::Rook, color};
        Piece queen{Piece::Type::Queen, color};
        Piece knight{Piece::Type::Knight, color};
        Piece king{Piece::Type::King, color};


#define AMBIG_MOVE_CHECK(fromCol, fromRow, toCol, toRow, pieceStr) \
    CHECK_MOVE_WITH_NAME(Move(fromCol, fromRow, toCol, toRow), pieceStr + Board::columnRowToSAN(toCol, toRow)); \
    board.setPiece(toCol, toRow, Piece{Piece::Type::Rook, opposite(color)}); \
    CHECK_MOVE_WITH_NAME(Move(fromCol, fromRow, toCol, toRow), pieceStr + ("x" + Board::columnRowToSAN(toCol, toRow))); \
    board.setPiece(toCol, toRow, std::nullopt)

        SECTION("Bishop") {
            SECTION("2 Bishops can move to square identify with col") {
                board.setPiece(0, 0, bishop);
                BoardIndex secondCoord = GENERATE(TEST_SOME(range(2, 8)));
                board.setPiece(secondCoord, secondCoord, bishop);

                std::string secondColLetter = Board::columnRowToSAN(secondCoord, 0).substr(0, 1);


                AMBIG_MOVE_CHECK(0, 0, 1, 1, "Ba");
                AMBIG_MOVE_CHECK(secondCoord, secondCoord, 1, 1, "B" + secondColLetter);
            }

            SECTION("2 Bishops on the same column are identified by row") {
                board.setPiece(4, 0, bishop);
                board.setPiece(4, 6, bishop);

                AMBIG_MOVE_CHECK(4, 0, 1, 3, "B1");
                AMBIG_MOVE_CHECK(4, 6, 1, 3, "B7");

                AMBIG_MOVE_CHECK(4, 0, 7, 3, "B1");
                AMBIG_MOVE_CHECK(4, 6, 7, 3, "B7");
            }

            SECTION("3 bishops which can move to the same square means one must be fully quilified") {
                board.setPiece(1, 2, bishop); // shares row
                board.setPiece(5, 2, bishop); // shares row and column
                board.setPiece(5, 6, bishop); // shares column

                AMBIG_MOVE_CHECK(1, 2, 3, 4, "Bb");
                AMBIG_MOVE_CHECK(5, 2, 3, 4, "Bf3");
                AMBIG_MOVE_CHECK(5, 6, 3, 4, "B7");
            }

            SECTION("4 bishops which can move to the same square means all must be fully quilified") {
                board.setPiece(1, 6, bishop);
                board.setPiece(1, 2, bishop);
                board.setPiece(5, 2, bishop);
                board.setPiece(5, 6, bishop);

                AMBIG_MOVE_CHECK(1, 6, 3, 4, "Bb7");
                AMBIG_MOVE_CHECK(1, 2, 3, 4, "Bb3");
                AMBIG_MOVE_CHECK(5, 2, 3, 4, "Bf3");
                AMBIG_MOVE_CHECK(5, 6, 3, 4, "Bf7");
            }

            SECTION("4 bishops when moving to square just 2 can reach only disambiguates on column") {
                board.setPiece(1, 6, bishop);
                board.setPiece(1, 2, bishop);
                board.setPiece(5, 2, bishop);
                board.setPiece(5, 6, bishop);

                AMBIG_MOVE_CHECK(1, 6, 2, 5, "Bb");
                AMBIG_MOVE_CHECK(5, 2, 2, 5, "Bf");
            }

        }

        SECTION("Rook") {
            SECTION("2 Rooks can move to the same square in col identify with row") {
                board.setPiece(0, 0, rook);
                BoardIndex secondCoord = GENERATE(TEST_SOME(range(2, 8)));
                board.setPiece(0, secondCoord, rook);

                std::string rowLetter = Board::columnRowToSAN(0, secondCoord).substr(1, 1);

                AMBIG_MOVE_CHECK(0, 0, 0, 1, "R1");
                AMBIG_MOVE_CHECK(0, secondCoord, 0, 1, "R" + rowLetter);
            }

            SECTION("2 Rooks which can move the same square from different rows") {
                board.setPiece(0, 0, rook);
                board.setPiece(4, 4, rook);

                AMBIG_MOVE_CHECK(0, 0, 4, 0, "Ra");
                AMBIG_MOVE_CHECK(4, 4, 4, 0, "Re");
            }

            SECTION("3 rooks around square") {
                board.setPiece(0, 4, rook);
                board.setPiece(4, 0, rook);
                board.setPiece(4, 7, rook);

                AMBIG_MOVE_CHECK(0, 4, 4, 4, "Ra");
                AMBIG_MOVE_CHECK(4, 0, 4, 4, "R1");
                AMBIG_MOVE_CHECK(4, 7, 4, 4, "R8");
            }

            SECTION("4 rooks around square") {
                board.setPiece(0, 4, rook);
                board.setPiece(4, 0, rook);
                board.setPiece(4, 7, rook);
                board.setPiece(7, 4, rook);

                AMBIG_MOVE_CHECK(0, 4, 4, 4, "Ra");
                AMBIG_MOVE_CHECK(4, 0, 4, 4, "R1");
                AMBIG_MOVE_CHECK(4, 7, 4, 4, "R8");
                AMBIG_MOVE_CHECK(7, 4, 4, 4, "Rh");
            }
        }

        SECTION("Knight") {
            SECTION("Two knights on different columns as disambiguated") {
                board.setPiece(1, 2, knight);
                board.setPiece(2, 1, knight);

                AMBIG_MOVE_CHECK(1, 2, 3, 3, "Nb");
                AMBIG_MOVE_CHECK(2, 1, 3, 3, "Nc");
            }

            SECTION("Two knights on same columns as disambiguated by row") {
                board.setPiece(1, 2, knight);
                board.setPiece(1, 4, knight);

                AMBIG_MOVE_CHECK(1, 2, 3, 3, "N3");
                AMBIG_MOVE_CHECK(1, 4, 3, 3, "N5");
            }

            SECTION("Can have 4 knights at different cols hitting same square") {
                board.setPiece(1, 2, knight);
                board.setPiece(2, 1, knight);
                board.setPiece(4, 5, knight);
                board.setPiece(5, 4, knight);

                AMBIG_MOVE_CHECK(1, 2, 3, 3, "Nb");
                AMBIG_MOVE_CHECK(2, 1, 3, 3, "Nc");
                AMBIG_MOVE_CHECK(4, 5, 3, 3, "Ne");
                AMBIG_MOVE_CHECK(5, 4, 3, 3, "Nf");
            }

            SECTION("Can have 4 knights at different rows hitting same square") {
                board.setPiece(1, 2, knight);
                board.setPiece(2, 1, knight);
                board.setPiece(1, 4, knight);
                board.setPiece(2, 5, knight);

                AMBIG_MOVE_CHECK(1, 2, 3, 3, "N3");
                AMBIG_MOVE_CHECK(2, 1, 3, 3, "N2");
                AMBIG_MOVE_CHECK(1, 4, 3, 3, "N5");
                AMBIG_MOVE_CHECK(2, 5, 3, 3, "N6");
            }
        }

        SECTION("Queens") {

            SECTION("2 queens on different columns are disambiguated by column") {
                board.setPiece(1, 1, queen);
                board.setPiece(3, 1, queen);

                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Qb");
                AMBIG_MOVE_CHECK(3, 1, 3, 3, "Qd");
            }

            SECTION("3 queens on different columns are disambiguated by column") {
                board.setPiece(1, 1, queen);
                board.setPiece(3, 1, queen);
                board.setPiece(5, 1, queen);

                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Qb");
                AMBIG_MOVE_CHECK(3, 1, 3, 3, "Qd");
                AMBIG_MOVE_CHECK(5, 1, 3, 3, "Qf");
            }

            SECTION("2 queens on same column are disambiguated by row") {
                board.setPiece(1, 1, queen);
                board.setPiece(1, 3, queen);

                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Q2");
                AMBIG_MOVE_CHECK(1, 3, 3, 3, "Q4");
            }

            SECTION("3 queens on same column are disambiguated by row") {
                board.setPiece(1, 1, queen);
                board.setPiece(1, 3, queen);
                board.setPiece(1, 5, queen);

                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Q2");
                AMBIG_MOVE_CHECK(1, 3, 3, 3, "Q4");
                AMBIG_MOVE_CHECK(1, 5, 3, 3, "Q6");
            }

            SECTION("3 queens in straight angle are disambiguated by row,full,col") {
                board.setPiece(1, 1, queen);
                board.setPiece(1, 3, queen);
                board.setPiece(3, 1, queen);

                AMBIG_MOVE_CHECK(1, 3, 3, 3, "Q4");
                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Qb2");
                AMBIG_MOVE_CHECK(3, 1, 3, 3, "Qd");
            }

            SECTION("5 queens in straight angle are disambiguated by row,row,full,col,col") {
                board.setPiece(1, 5, queen);
                board.setPiece(1, 3, queen);
                board.setPiece(1, 1, queen);
                board.setPiece(3, 1, queen);
                board.setPiece(5, 1, queen);

                AMBIG_MOVE_CHECK(1, 5, 3, 3, "Q6");
                AMBIG_MOVE_CHECK(1, 3, 3, 3, "Q4");
                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Qb2");
                AMBIG_MOVE_CHECK(3, 1, 3, 3, "Qd");
                AMBIG_MOVE_CHECK(5, 1, 3, 3, "Qf");
            }

            SECTION("5 queens in U form are disambiguated") {
                board.setPiece(1, 5, queen);
                board.setPiece(1, 3, queen);
                board.setPiece(1, 1, queen);
                board.setPiece(3, 1, queen);
                board.setPiece(3, 5, queen);

                AMBIG_MOVE_CHECK(1, 5, 3, 3, "Qb6");
                AMBIG_MOVE_CHECK(1, 3, 3, 3, "Q4");
                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Qb2");
                AMBIG_MOVE_CHECK(3, 1, 3, 3, "Qd2");
                AMBIG_MOVE_CHECK(3, 5, 3, 3, "Qd6");
            }

            SECTION("8 queens around square all need to be fully disambiguated") {
                board.setPiece(1, 5, queen);
                board.setPiece(1, 3, queen);
                board.setPiece(1, 1, queen);
                board.setPiece(3, 1, queen);
                board.setPiece(5, 1, queen);
                board.setPiece(5, 3, queen);
                board.setPiece(5, 5, queen);
                board.setPiece(3, 5, queen);

                AMBIG_MOVE_CHECK(1, 5, 3, 3, "Qb6");
                AMBIG_MOVE_CHECK(1, 3, 3, 3, "Qb4");
                AMBIG_MOVE_CHECK(1, 1, 3, 3, "Qb2");
                AMBIG_MOVE_CHECK(3, 1, 3, 3, "Qd2");
                AMBIG_MOVE_CHECK(5, 1, 3, 3, "Qf2");
                AMBIG_MOVE_CHECK(5, 3, 3, 3, "Qf4");
                AMBIG_MOVE_CHECK(5, 5, 3, 3, "Qf6");
                AMBIG_MOVE_CHECK(3, 5, 3, 3, "Qd6");
            }

        }

        SECTION("Legality") {

            SECTION("No ambiguity if bishop behind other") {
                board.setPiece(0, 0, bishop);
                board.setPiece(1, 1, bishop);

                AMBIG_MOVE_CHECK(1, 1, 2, 2, "B");
            }

            SECTION("Queen cannot move through piece thus no disambiguation") {
                board.setPiece(0, 0, queen);
                board.setPiece(0, 1, knight);
                board.setPiece(2, 0, queen);

                // in case order matters we check both
                AMBIG_MOVE_CHECK(2, 0, 0, 2, "Q");
                board.setPiece(0, 1, std::nullopt);
                board.setPiece(1, 1, knight);
                AMBIG_MOVE_CHECK(0, 0, 0, 2, "Q");
            }

            SECTION("Rooks cannot move through piece thus no disambiguation") {
                board.setPiece(0, 0, rook);
                board.setPiece(5, 0, rook);

                board.setPiece(3, 0, knight);
                AMBIG_MOVE_CHECK(0, 0, 2, 0, "R");
                AMBIG_MOVE_CHECK(5, 0, 4, 0, "R");
            }

            SECTION("4 bishops to squares where a single bishop can move are not disambiguated") {
                board.setPiece(1, 6, bishop);
                board.setPiece(1, 2, bishop);
                board.setPiece(5, 2, bishop);
                board.setPiece(5, 6, bishop);

                AMBIG_MOVE_CHECK(1, 6, 2, 7, "B");
                AMBIG_MOVE_CHECK(1, 2, 2, 1, "B");
                AMBIG_MOVE_CHECK(5, 2, 4, 1, "B");
                AMBIG_MOVE_CHECK(5, 6, 6, 5, "B");
            }

            SECTION("2 Rooks which cannot move to the square are not disambiguated") {
                board.setPiece(0, 0, rook);
                board.setPiece(4, 4, rook);

                AMBIG_MOVE_CHECK(0, 0, 3, 0, "R");
                AMBIG_MOVE_CHECK(4, 4, 4, 1, "R");
            }

            SECTION("Pinned rook") {
                board.setPiece(0, 0, king);
                board.setPiece(0, 1, rook); // pinned 1
                board.setPiece(0, 7, Piece{Piece::Type::Rook, opposite(color)});
                board.setPiece(1, 0, rook); // piece 2

                AMBIG_MOVE_CHECK(1, 0, 1, 1, "R");
                board.setPiece(0, 7, std::nullopt);
                board.setPiece(7, 0, Piece{Piece::Type::Rook, opposite(color)});
                // now pinning 2 in case of lucky order
                AMBIG_MOVE_CHECK(0, 1, 1, 1, "R");
            }

        }

        // 4 bishops can take rook
        // one needs full disam one just col one just row
        // B4k2/5B2/8/3r4/8/1B3B2/8/4K3 w - - 0 1

#undef AMBIG_MOVE_CHECK
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

        SECTION("Take en passant") {
            BoardIndex col = GENERATE(TEST_SOME(range(0, 8)));
            Color color = GENERATE(Color::White, Color::Black);

            Board board = TestUtil::createEnPassantBoard(color, col);

            BoardIndex myCol = col +
                               GENERATE_COPY(filter([col](int8_t i) {
                                 return col + i >= 0 && col + i < 8;
                               }, values({1, -1})));

            std::string myColLetter = Board::columnRowToSAN(myCol, 0).substr(0, 1);
            CAPTURE(col, myCol);

            BoardIndex behindPawn = Board::pawnHomeRow(opposite(color)) + Board::pawnDirection(opposite(color));
            BoardIndex enPassantRow =  behindPawn + Board::pawnDirection(opposite(color));

            Piece pawn = Piece{Piece::Type::Pawn, color};
            board.setPiece(myCol, enPassantRow, pawn);

            Move move = {myCol, enPassantRow, col, behindPawn, Move::Flag::EnPassant};
            CHECK_MOVE_WITH_NAME(move, myColLetter + "x" + Board::columnRowToSAN(col, behindPawn));
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

TEST_CASE("Long Move format", "[chess][san][parsing]") {

#define MOVE_HAS_NAME(move, str) \
    REQUIRE((move).toSANSquares() == str)

    SECTION("Basic move correctness") {
#define SIMPLE_MOVE(from, to) \
        MOVE_HAS_NAME(Move(from, to), from to)
        SIMPLE_MOVE("a1", "a2");
        SIMPLE_MOVE("b1", "a2");
        SIMPLE_MOVE("d1", "a7");
        SIMPLE_MOVE("e1", "e5");
        SIMPLE_MOVE("e2", "e4");
    }

    SECTION("Castling") {
        Color c = GENERATE(Color::White, Color::Black);
        std::string row = c == Color::White ? "1" : "8";
        MOVE_HAS_NAME(Move(Board::kingCol, Board::homeRow(c), Board::queenSideRookCol, Board::homeRow(c), Move::Flag::Castling), "e" + row + "c" + row);
        MOVE_HAS_NAME(Move(Board::kingCol, Board::homeRow(c), Board::kingSideRookCol, Board::homeRow(c), Move::Flag::Castling), "e" + row + "g" + row);
    }

    SECTION("Promotion") {
        Color c = GENERATE(Color::White, Color::Black);
        char col = GENERATE(TEST_SOME(range('a', char('h' + 1))));
        std::string from = col + std::to_string(Board::pawnHomeRow(opposite(c)) + 1);
        std::string to = col + std::to_string(Board::homeRow(opposite(c)) + 1);

        MOVE_HAS_NAME(Move(from, to, Move::Flag::PromotionToQueen), from + to + "q");
        MOVE_HAS_NAME(Move(from, to, Move::Flag::PromotionToBishop), from + to + "b");
        MOVE_HAS_NAME(Move(from, to, Move::Flag::PromotionToKnight), from + to + "n");
        MOVE_HAS_NAME(Move(from, to, Move::Flag::PromotionToRook), from + to + "r");
    }

}
