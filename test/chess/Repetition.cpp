#include "TestUtil.h"
#include <catch2/catch_test_macros.hpp>
#include <chess/Board.h>

TEST_CASE("Can detect repetition") {

    using namespace Chess;

    SECTION("From starting position") {
        Board board = Board::standardBoard();

        REQUIRE(board.positionRepeated() == 0);

        Move whiteMove1{"b1", "a3"};
        Move whiteMove2{"a3", "b1"};

        Move blackMove1{"g8", "h6"};
        Move blackMove2{"h6", "g8"};

// Any of the boards should all have occurred repeated times in history
#define REPEAT_MOVE(times) \
    do {                   \
       auto repeated = board.positionRepeated(); \
    for (int i = repeated; i < repeated + (times); ++i) {\
        CAPTURE(i);\
        REQUIRE(board.positionRepeated() == i);\
        board.makeMove(whiteMove1);\
        REQUIRE(board.positionRepeated() == i);\
        board.makeMove(whiteMove2);\
        REQUIRE(board.positionRepeated() == i);\
        board.makeMove(blackMove1);\
        REQUIRE(board.positionRepeated() == i);\
        board.makeMove(blackMove2);\
        REQUIRE(board.positionRepeated() == i + 1);\
    } \
    } while(false)

        SECTION("Same knight moves") {
            REPEAT_MOVE(10);
            REQUIRE(board.positionRepeated() == 10);
        }

        SECTION("Pawn move breaks repetition") {
            Move mv{"e2", "e4", Move::Flag::DoublePushPawn};
            Move mv2{"e7", "e5", Move::Flag::DoublePushPawn};

            REPEAT_MOVE(2);

            REQUIRE(board.positionRepeated() == 2);


            board.makeMove(mv);
            REQUIRE(board.positionRepeated() == 0);
            board.makeMove(mv2);
            REQUIRE(board.positionRepeated() == 0);
            REQUIRE(board.enPassantColRow().has_value());


            REPEAT_MOVE(2);
            REQUIRE(board.positionRepeated() == 2);
        }
    }


}
