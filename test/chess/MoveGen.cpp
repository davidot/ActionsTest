#include <catch2/catch.hpp>
#include <chess/MoveGen.h>

TEST_CASE("Move generation", "[chess][movegen]") {
    using namespace Chess;

    SECTION("Empty board has no moves") {
        Board board = Board::emptyBoard();
        MoveList list = generateAllMoves(board);
        REQUIRE(list.size() == 0);
    }

    SECTION("Board with only colors not in turn has no moves") {
        Board board = Board::emptyBoard();
        Color c = opposite(board.colorToMove());
        Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), c};
        board.setPiece(4, 4, p);
        MoveList list = generateAllMoves(board);
        REQUIRE(list.size() == 0);
        list.forEachMove([](const auto&){
            //Should not be called
            REQUIRE(false);
        });
    }

    SECTION("Invalid boards with single piece still generate moves") {

        Board board = Board::emptyBoard();
        Color toMove = Color::White;
        [[maybe_unused]] Color other = opposite(Color::White); // TODO use or remove

        auto validateCountAndFrom = [](const MoveList& list, Board::BoardIndex col, Board::BoardIndex row, unsigned count) {
          REQUIRE(list.size() == count);
          unsigned calls = 0;

          auto index = Board::columnRowToIndex(col, row);
          list.forEachMove([&](const Move& move) {
            calls++;
            CAPTURE(move);
            REQUIRE(move.fromPosition == index);
            REQUIRE(move.toPosition != index);
          });
          REQUIRE(calls == count);

          list.forEachMoveFrom(col, row, [&](const Move& move) {
            calls--;
            CAPTURE(move);
            REQUIRE(move.fromPosition == index);
            REQUIRE(move.toPosition != index);
          });

          REQUIRE(calls == 0);
        };


        SECTION("Board with single pawn has just one move") {
            board.setPiece(4, 4, Piece(Piece::Type::Pawn, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, 4, 4, 1);
        }

        SECTION("Board with single rook always has 14 moves") {
            uint8_t col = GENERATE(range(0u, 8u));
            uint8_t row = GENERATE(range(0u, 8u));
            board.setPiece(col, row, Piece(Piece::Type::Rook, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, col, row, 14);
        }

        SECTION("Bishop in the center has 13 moves") {
            uint8_t col = GENERATE(3, 4);
            uint8_t row = GENERATE(3, 4);
            board.setPiece(col, row, Piece(Piece::Type::Bishop, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, col, row, 13);
        }

        SECTION("Bishop in the corner has 7 moves") {
            uint8_t col = GENERATE(0, 7);
            uint8_t row = GENERATE(0, 7);
            board.setPiece(col, row, Piece(Piece::Type::Bishop, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, col, row, 7);
        }

        SECTION("King in the center has 8 moves") {
            board.setPiece(4, 4, Piece(Piece::Type::King, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, 4, 4, 8);
        }

        SECTION("Knight in the center has 8 moves") {
            board.setPiece(4, 4, Piece(Piece::Type::Knight, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, 4, 4, 8);
        }

        SECTION("Queen in the center has 14+13 moves") {
            uint8_t col = GENERATE(3, 4);
            uint8_t row = GENERATE(3, 4);
            board.setPiece(col, row, Piece(Piece::Type::Queen, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, col, row, 14 + 13);
        }

    }

    SECTION("Multiple pieces (same color still)") {
        Board board = Board::emptyBoard();
        Color toMove = Color::White;
        [[maybe_unused]] Color other = opposite(Color::White); // TODO use or remove

        SECTION("Board with multiple pawns has multiple moves") {
            uint8_t count = GENERATE(range(2u, 8u));
            for (uint8_t i = 0; i < count; i++) {
                board.setPiece(i, 4, Piece(Piece::Type::Pawn, toMove));
            }
            MoveList list = generateAllMoves(board);
            REQUIRE(list.size() == count);
        }
    }
}
