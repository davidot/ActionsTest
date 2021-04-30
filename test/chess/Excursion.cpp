#include "TestUtil.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chess/Board.h>


using namespace Chess;

TEST_CASE("Excursions") {
    Board board = Board::standardBoard();

    const Board& constBoard = board;

    SECTION("Excursion works on simple move") {
        std::string fromSquare = "b1";
        std::string toSquare = "a3";
        Move whiteMove1{fromSquare, toSquare};
        Piece knight{Piece::Type::Knight, Color::White};

        REQUIRE(board.pieceAt(fromSquare) == knight);
        REQUIRE(board.pieceAt(toSquare) == std::nullopt);

        constBoard.moveExcursion(whiteMove1, [&](const Board& callbackBoard){
            REQUIRE(callbackBoard.pieceAt(fromSquare) == std::nullopt);
            REQUIRE(callbackBoard.pieceAt(toSquare) == knight);
        });

        REQUIRE(board.pieceAt(fromSquare) == knight);
        REQUIRE(board.pieceAt(toSquare) == std::nullopt);
    }

    SECTION("Excursion returns value 'calculated' on simple move") {
        std::string fromSquare = "b1";
        std::string toSquare = "a3";
        Move whiteMove1{fromSquare, toSquare};
        Piece knight{Piece::Type::Knight, Color::White};

        int val = GENERATE(TEST_SOME(values({0, -1, 1, 24, 43, -192})));

        REQUIRE(board.pieceAt(fromSquare) == knight);
        REQUIRE(board.pieceAt(toSquare) == std::nullopt);


        auto callback = [&](const Board& callbackBoard) {
            REQUIRE(callbackBoard.pieceAt(fromSquare) == std::nullopt);
            REQUIRE(callbackBoard.pieceAt(toSquare) == knight);
            return val;
        };

        static_assert(std::is_same_v<
                decltype(constBoard.moveExcursion(whiteMove1, callback)), decltype(val)
              >,
              "Return type must be exactly returned type");

        int result = constBoard.moveExcursion(whiteMove1, callback);

        REQUIRE(result == val);

        REQUIRE(board.pieceAt(fromSquare) == knight);
        REQUIRE(board.pieceAt(toSquare) == std::nullopt);
    }

    SECTION("Can have recursive excursions") {
        std::string fromSquare = "b1";
        std::string toSquare = "a3";
        Move whiteMove1{fromSquare, toSquare};

        std::string fromSquare2 = "g8";
        std::string toSquare2 = "h6";
        Move blackMove1{fromSquare2, toSquare2};

        Piece knight{Piece::Type::Knight, Color::White};
        Piece bKnight{Piece::Type::Knight, Color::Black};

        REQUIRE(board.pieceAt(fromSquare) == knight);
        REQUIRE(board.pieceAt(toSquare) == std::nullopt);
        REQUIRE(board.pieceAt(fromSquare2) == bKnight);
        REQUIRE(board.pieceAt(toSquare2) == std::nullopt);


        int val = GENERATE(TEST_SOME(values({0, -1, 1, 24, 43, -192})));

        double val2 = GENERATE(TEST_SOME(values({0.0, 3.0, -2.1, 9.875})));

        auto callback2 = [&](const Board& callbackBoard) {
          REQUIRE(callbackBoard.pieceAt(fromSquare) == std::nullopt);
          REQUIRE(callbackBoard.pieceAt(toSquare) == knight);

          REQUIRE(board.pieceAt(fromSquare2) == std::nullopt);
          REQUIRE(board.pieceAt(toSquare2) == bKnight);
          return val2;
        };

        auto callback1 = [&](const Board& callbackBoard) {
          REQUIRE(callbackBoard.pieceAt(fromSquare) == std::nullopt);
          REQUIRE(callbackBoard.pieceAt(toSquare) == knight);

          REQUIRE(board.pieceAt(fromSquare2) == bKnight);
          REQUIRE(board.pieceAt(toSquare2) == std::nullopt);

          auto returned = callbackBoard.moveExcursion(blackMove1, callback2);

          static_assert(std::is_same_v<decltype(returned), decltype(val2)>);

          static_assert(std::is_same_v<
                                decltype(constBoard.moveExcursion(blackMove1, callback2)), decltype(val2)
                        >,
                        "Return type must be exactly returned type");

          REQUIRE(callbackBoard.pieceAt(fromSquare) == std::nullopt);
          REQUIRE(callbackBoard.pieceAt(toSquare) == knight);

          REQUIRE(board.pieceAt(fromSquare2) == bKnight);
          REQUIRE(board.pieceAt(toSquare2) == std::nullopt);

          return val;
        };

        static_assert(std::is_same_v<
                              decltype(constBoard.moveExcursion(whiteMove1, callback1)), decltype(val)
                      >,
                      "Return type must be exactly returned type");

        int result = constBoard.moveExcursion(whiteMove1, callback1);

        REQUIRE(result == val);

        REQUIRE(board.pieceAt(fromSquare) == knight);
        REQUIRE(board.pieceAt(toSquare) == std::nullopt);
        REQUIRE(board.pieceAt(fromSquare2) == bKnight);
        REQUIRE(board.pieceAt(toSquare2) == std::nullopt);
    }

}
