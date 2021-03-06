#include "TestUtil.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <chess/Board.h>

TEST_CASE("Can detect repetition", "[chess][legal]") {

    using namespace Chess;

    SECTION("From starting position") {
        Board board = Board::standardBoard();

        REQUIRE(board.positionRepeated() == 0);

        Move whiteMove1{"b1", "a3"};
        Move whiteMove2{"a3", "b1"};

        Move blackMove1{"g8", "h6"};
        Move blackMove2{"h6", "g8"};

// Any of the boards should all have occurred repeated times in history
#define REPEAT_MOVE(times)                   \
    do {                                     \
        for (auto i = 0u; i < (times); ++i) { \
            board.makeMove(whiteMove1);      \
            board.makeMove(blackMove1);      \
            board.makeMove(whiteMove2);      \
            board.makeMove(blackMove2);      \
        }                                    \
    } while (false)

        SECTION("Single repetition") {
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(whiteMove1);
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(blackMove1);
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(whiteMove2);
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(blackMove2);
            REQUIRE(board.positionRepeated() == 1);
        }

        SECTION("Same knight moves") {
            // test some generate range
            size_t times = GENERATE(TEST_SOME(range(1, 11)));
            CAPTURE(times);
            REPEAT_MOVE(times);
            REQUIRE(board.positionRepeated() == times);
            for (int i = 0; i < 3; i++) {
                CAPTURE(i);
                board.undoMove();
                REQUIRE(board.positionRepeated() == times - 1);
            }
            if (times > 3) {
                SECTION("Gives possible draw") {
                    REQUIRE(board.isDrawn());
                }
            }
            if (times > 5) {
                SECTION("Gives forced draw") {
                    REQUIRE(board.isDrawn(true));
                }
            }
        }

        SECTION("Position with en passant differs from no en passant") {
            board.makeMove({"e2", "e4", Move::Flag::DoublePushPawn});
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove(blackMove1);
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(whiteMove1);
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(blackMove2);
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(whiteMove2);
            REQUIRE(board.positionRepeated() == 0);
        }

        SECTION("CastlingRights change breaks repetition") {
            board.setPiece(Board::queenSideRookCol, Board::pawnHomeRow(Color::White), std::nullopt);

            REQUIRE(board.positionRepeated() == 0);
            board.makeMove({Board::queenSideRookCol, Board::homeRow(Color::White), Board::queenSideRookCol, Board::pawnHomeRow(Color::White)});
            REQUIRE(board.positionRepeated() == 0);
            CHECK(board.castlingRights() != CastlingRight::AnyCastling);

            board.makeMove(blackMove1);
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove({Board::queenSideRookCol, Board::pawnHomeRow(Color::White), Board::queenSideRookCol, Board::homeRow(Color::White)});
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove(blackMove2);
            // 0 since castling rights are different!
            REQUIRE(board.positionRepeated() == 0);
        }

        SECTION("Repetition holds even with moves in between") {
            REPEAT_MOVE(3);
            REQUIRE(board.positionRepeated() == 3);

            board.makeMove({"g1", "h3"});
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove({"b8", "a6"});
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove({"h3", "g1"});
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove({"a6", "b8"});
            REQUIRE(board.positionRepeated() == 4);

            REPEAT_MOVE(3);
            REQUIRE(board.positionRepeated() == 7);
            SECTION("Is drawn position") {
                REQUIRE(board.isDrawn());
                REQUIRE(board.isDrawn(true));
            }
        }

        SECTION("Castling breaks repetition (both by rights and non reversible)") {
            Color c = GENERATE(Color::White, Color::Black);
            Color opp = opposite(c);

            board = TestUtil::generateCastlingBoard(c, true, true, false, true);
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove({Board::kingCol, Board::homeRow(c), Board::kingSideRookCol, Board::homeRow(c), Move::Flag::Castling});
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove({Board::queenSideRookCol, Board::homeRow(opp), Board::queenSideRookCol, Board::pawnHomeRow(opp)});
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove({Board::queenSideRookCol, Board::homeRow(c), Board::queenSideRookCol, Board::pawnHomeRow(c)});
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove({Board::queenSideRookCol, Board::pawnHomeRow(opp), Board::queenSideRookCol, Board::homeRow(opp)});
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove({Board::queenSideRookCol, Board::pawnHomeRow(c), Board::queenSideRookCol, Board::homeRow(c)});
            REQUIRE(board.positionRepeated() == 0);

            board.makeMove({Board::queenSideRookCol, Board::homeRow(opp), Board::queenSideRookCol, Board::pawnHomeRow(opp)});
            REQUIRE(board.positionRepeated() == 1);
        }

        SECTION("Gives draw for repetition (strict after 5)") {
            REPEAT_MOVE(4);
            REQUIRE_FALSE(board.isDrawn(true));
            REPEAT_MOVE(1);
            REQUIRE(board.isDrawn(true));
        }

        SECTION("Gives draw for repetition (possible after 3)") {
            REPEAT_MOVE(2);
            REQUIRE_FALSE(board.isDrawn());
            REPEAT_MOVE(1);
            REQUIRE(board.isDrawn());
        }
    }



    // TODO: define/test null moves behavior


}
