#include <catch2/catch.hpp>
#include <chess/MoveGen.h>

#define REQUIRE_EMPTY(list) \
    REQUIRE(list.size() == 0); \
    list.forEachMove([](const auto&){ \
        /*Should not be called*/ \
        REQUIRE(false); \
    })

TEST_CASE("Basic move checks", "[chess][move]") {
    using namespace Chess;
    REQUIRE(isPromotion(Move::Flags::PromotionToKnight));
    REQUIRE(isPromotion(Move::Flags::PromotionToBishop));
    REQUIRE(isPromotion(Move::Flags::PromotionToRook));
    REQUIRE(isPromotion(Move::Flags::PromotionToQueen));
    REQUIRE_FALSE(isPromotion(Move::Flags::None));
    REQUIRE_FALSE(isPromotion(Move::Flags::Castling));
    REQUIRE_FALSE(isPromotion(Move::Flags::DoublePushPawn));
    REQUIRE_FALSE(isPromotion(Move::Flags::EnPassant));
}

TEST_CASE("Move generation", "[chess][movegen]") {
    using namespace Chess;

    SECTION("Empty board has no moves") {
        Board board = Board::emptyBoard();
        MoveList list = generateAllMoves(board);
        REQUIRE_EMPTY(list);
    }

    SECTION("Board with only colors not in turn has no moves") {
        Board board = Board::emptyBoard();
        Color c = opposite(board.colorToMove());
        Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), c};
        board.setPiece(4, 4, p);
        MoveList list = generateAllMoves(board);
        REQUIRE_EMPTY(list);
    }

    SECTION("Invalid boards with single piece still generate moves") {
        Board board = Board::emptyBoard();
        if (GENERATE(true, false)) {
            board.makeNullMove();
        }
        Color toMove = board.colorToMove();
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
            REQUIRE(move.flags == Move::Flags::None);
          });
          REQUIRE(calls == count);

          list.forEachMoveFrom(col, row, [&](const Move& move) {
            calls--;
            CAPTURE(move);
            REQUIRE(move.fromPosition == index);
            REQUIRE(move.toPosition != index);
            REQUIRE(move.flags == Move::Flags::None);
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

    SECTION("In invalid board multiple pieces (same color still)") {
        Board board = Board::emptyBoard();
        if (GENERATE(true, false)) {
            board.makeNullMove();
        }
        Color toMove = board.colorToMove();
        [[maybe_unused]] Color other = opposite(Color::White); // TODO use or remove

        SECTION("Board with multiple pawns has multiple moves") {
            uint8_t count = GENERATE(range(2u, 8u));
            for (uint8_t i = 0; i < count; i++) {
                board.setPiece(i, 4, Piece(Piece::Type::Pawn, toMove));
            }
            MoveList list = generateAllMoves(board);
            REQUIRE(list.size() == count);
        }

        SECTION("Column containing pawns with nothing or another pawn above generates no moves") {
            if (toMove != Color::White) {
                return;
            }
            uint8_t col = GENERATE(range(0u, 8u));
            uint8_t count = GENERATE(range(0u, 8u));

            for (uint8_t i = 0; i < count; i++) {
                board.setPiece(col, 7 - i, Piece(Piece::Type::Pawn, toMove));
            }
            MoveList list = generateAllMoves(board);
            REQUIRE_EMPTY(list);
        }

        SECTION("Diagonal row of rooks all have 14 moves") {
            for (uint8_t i = 0; i < board.size(); i++) {
                board.setPiece(i, i, Piece(Piece::Type::Rook, toMove));
            }
            MoveList list = generateAllMoves(board);
            REQUIRE(list.size() == 14 * board.size());
            for (uint8_t i = 0; i < board.size(); i++) {
                unsigned count = 0;
                list.forEachMoveFrom(i, i, [&](const Move& move) {
                    count++;
                    auto [colFrom, rowFrom] = Board::indexToColumnRow(move.fromPosition);
                    auto [colTo, rowTo] = Board::indexToColumnRow(move.fromPosition);

                    REQUIRE((colTo == colFrom || rowTo == rowFrom));
                    REQUIRE(move.fromPosition != move.toPosition);
                    REQUIRE(move.flags == Move::Flags::None);
                });
                REQUIRE(count == 14);
            }
        }
    }

    SECTION("Pieces are blocked by pieces of the same color") {
        Board board = Board::emptyBoard();
        if (GENERATE(true, false)) {
            board.makeNullMove();
        }
        Color toMove = board.colorToMove();
        [[maybe_unused]] Color other = opposite(Color::White); // TODO use or remove

        SECTION("Full board with any piece generates no moves") {
            Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), toMove};
            for (uint8_t col = 0; col < 8; col++) {
                for (uint8_t row = 0; row < 8; row++) {
                    board.setPiece(col, row, p);
                }
            }
            MoveList list = generateAllMoves(board);
            REQUIRE_EMPTY(list);
        }

        SECTION("All pieces (except knight) trapped in corner cannot move") {
            // No knight
            board.setPiece(0, 0, Piece{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), toMove});

            Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), toMove};
            board.setPiece(0, 1, p);
            board.setPiece(1, 0, p);
            board.setPiece(1, 1, p);

            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(0, 0, [&](const Move& move) {
              REQUIRE(false);
            });
        }

        SECTION("Knights can jump over pieces") {
            board.setPiece(0, 0, Piece{Piece::Type::Knight, toMove});

            Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), GENERATE(Color::White, Color::Black)};
            board.setPiece(0, 1, p);
            board.setPiece(1, 0, p);
            board.setPiece(1, 1, p);

            MoveList list = generateAllMoves(board);
            unsigned count = 0;
            list.forEachMoveFrom(0, 0, [&](const Move& move) {
                count++;
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flags == Move::Flags::None);
            });
            REQUIRE(count == 2);
        }
    }

    SECTION("Does generate capturing moves") {
        Board board = Board::emptyBoard();
        if (GENERATE(true, false)) {
            board.makeNullMove();
        }
        Color toMove = board.colorToMove();
        Color other = opposite(toMove);

        CAPTURE(toMove);

        SECTION("Queen and King can capture all pieces around it") {
            Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), other};
            for (uint8_t col = 3; col <= 5; col++) {
                for (uint8_t row = 3; row <= 5; row++) {
                    board.setPiece(col, row, p);
                }
            }

            board.setPiece(4, 4, Piece{GENERATE(Piece::Type::Queen, Piece::Type::King), toMove});

            MoveList list = generateAllMoves(board);
            unsigned count = 0;
            list.forEachMoveFrom(4, 4, [&](const Move& move) {
                count++;
                REQUIRE(move.toPosition != move.fromPosition);
                auto [col, row] = Board::indexToColumnRow(move.toPosition);
                CAPTURE(col, row);
                REQUIRE(board.pieceAt(col, row) == p);
            });
            REQUIRE(count == 8);
        }

        SECTION("Queen can capture pieces far away") {
            Piece p{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), other};
            board.setPiece(0, 0, p);
            board.setPiece(1, 4, p);
            board.setPiece(2, 6, p);
            board.setPiece(4, 7, p);
            board.setPiece(5, 5, p);
            board.setPiece(7, 4, p);
            board.setPiece(7, 1, p);
            board.setPiece(4, 0, p);

            board.setPiece(4, 4, Piece{Piece::Type::Queen, toMove});

            MoveList list = generateAllMoves(board);
            unsigned captures = 0;
            list.forEachMoveFrom(4, 4, [&](const Move& move) {
              REQUIRE(move.toPosition != move.fromPosition);
              auto [col, row] = Board::indexToColumnRow(move.toPosition);
              CAPTURE(col, row);
              auto optPiece = board.pieceAt(col, row);
              if (optPiece) {
                  captures++;
                  REQUIRE(optPiece == p);
              }

            });
            REQUIRE(captures == 8);
        }

        SECTION("Queen cannot capture pieces if blocked by own color") {
            // fill board
            Piece capturable{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), other};
            Piece blocker{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), toMove};
            for (uint8_t col = 0; col < 8; col++) {
                for (uint8_t row = 0; row < 8; row++) {
                    if (col >= 2 && col <= 6 && row >= 2 && row <= 6) {
                        if (col == 2 || col == 6 || row == 2 || row == 6) {
                            board.setPiece(col, row, blocker);
                        }
                    } else {
                        board.setPiece(col, row, capturable);
                    }
                }
            }

            board.setPiece(4, 4, Piece{Piece::Type::Queen, toMove});

            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(4, 4, [&](const Move& move) {
              REQUIRE(move.toPosition != move.fromPosition);
              auto [col, row] = Board::indexToColumnRow(move.toPosition);
              CAPTURE(col, row);
              REQUIRE(board.pieceAt(col, row) == std::nullopt);
            });
        }

        SECTION("Pawns") {
            SECTION("Pawns cannot capture anywhere (except direct diagonals)") {
                Piece capturable{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), other};
                for (uint8_t col = 0; col < 8; col++) {
                    for (uint8_t row = 0; row < 8; row++) {
                        if ((col != 3 && col != 5) || (row != 5 && row != 3)) {
                            board.setPiece(col, row, capturable);
                        }
                    }
                }

                board.setPiece(4, 4, Piece{Piece::Type::Pawn, toMove});

                MoveList list = generateAllMoves(board);
                // (in fact we cannot even move!)
                REQUIRE_EMPTY(list);
            }

            SECTION("Pawns can capture forward and to the side (non en passant)") {
                Piece capturable{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), other};
                uint8_t col = GENERATE(3, 5);
                board.setPiece(col, 3, capturable);
                board.setPiece(col, 5, capturable);

                bool blocked = GENERATE(true, false);
                CAPTURE(blocked, col);
                if (blocked) {
                    Color c = GENERATE_COPY(toMove, other);
                    Piece moveBlocker{GENERATE(Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King), c};
                    board.setPiece(4, 3, moveBlocker);
                    board.setPiece(4, 5, moveBlocker);
                }

                board.setPiece(4, 4, Piece{Piece::Type::Pawn, toMove});

                MoveList list = generateAllMoves(board);
                unsigned captures = 0;
                list.forEachMoveFrom(4, 4, [&](const Move& move) {
                  REQUIRE(move.toPosition != move.fromPosition);
                  auto [col, row] = Board::indexToColumnRow(move.toPosition);
                  CAPTURE(col, row);
                  auto optPiece = board.pieceAt(col, row);
                  if (optPiece) {
                      captures++;
                      REQUIRE(optPiece == capturable);
                  }
                });
                REQUIRE(captures == 1);
            }

            SECTION("En passant") {
                // TODO: oh oh gonna be hard!
            }
        }
    }
}
