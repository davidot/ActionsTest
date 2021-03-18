#include <catch2/catch.hpp>
#include <chess/MoveGen.h>
#include <set>
#include "TestUtil.h"

#define REQUIRE_EMPTY(list) \
    REQUIRE(list.size() == 0); \
    list.forEachMove([](const auto&){ \
        /*Should not be called*/ \
        REQUIRE(false); \
    })

TEST_CASE("Basic move checks", "[chess][move]") {
    using namespace Chess;

    auto isPromo = [](Move::Flag flag) {
        Move mv = Move{0, 0, flag};
        return mv.isPromotion();
    };

    REQUIRE(isPromo(Move::Flag::PromotionToKnight));
    REQUIRE(isPromo(Move::Flag::PromotionToBishop));
    REQUIRE(isPromo(Move::Flag::PromotionToRook));
    REQUIRE(isPromo(Move::Flag::PromotionToQueen));
    REQUIRE_FALSE(isPromo(Move::Flag::None));
    REQUIRE_FALSE(isPromo(Move::Flag::Castling));
    REQUIRE_FALSE(isPromo(Move::Flag::DoublePushPawn));
    REQUIRE_FALSE(isPromo(Move::Flag::EnPassant));

    auto promoType = [](Move::Flag flag) {
      Move mv = Move{0, 0, flag};
      return mv.promotedType();
    };

    REQUIRE(promoType(Chess::Move::Flag::PromotionToKnight) == Piece::Type::Knight);
    REQUIRE(promoType(Chess::Move::Flag::PromotionToBishop) == Piece::Type::Bishop);
    REQUIRE(promoType(Chess::Move::Flag::PromotionToRook) == Piece::Type::Rook);
    REQUIRE(promoType(Chess::Move::Flag::PromotionToQueen) == Piece::Type::Queen);
}

// technically it is not valid to capture a king so lets not depend that being possible here
#define CAPTURABLE_TYPES TEST_SOME(values({Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen}))
#define ANY_TYPE TEST_SOME(values({Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen, Piece::Type::King}))


TEST_CASE("Move generation basic", "[chess][rules][movegen]") {
    using namespace Chess;

    SECTION("Empty board has no moves") {
        Board board = Board::emptyBoard();
        MoveList list = generateAllMoves(board);
        REQUIRE_EMPTY(list);
    }

    SECTION("Board with only colors not in turn has no moves") {
        Board board = Board::emptyBoard();
        Color c = opposite(board.colorToMove());
        Piece p{GENERATE(ANY_TYPE), c};
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
        [[maybe_unused]] Color other = opposite(Color::White);// TODO use or remove


        auto validateCountAndFrom = [](const MoveList &list, Board::BoardIndex col, Board::BoardIndex row, unsigned count) {
            REQUIRE(list.size() == count);
            std::set<Board::BoardIndex> destinations;

            list.forEachMove([&](const Move &move) {
                CAPTURE(move);
                auto [colFrom, rowFrom] = move.colRowFromPosition();
                REQUIRE(colFrom == col);
                REQUIRE(rowFrom == row);
                REQUIRE(move.toPosition != move.fromPosition);
                REQUIRE(move.flag == Move::Flag::None);
                destinations.insert(move.toPosition);
            });
            REQUIRE(destinations.size() == count);

            unsigned calls = 0;
            list.forEachMoveFrom(col, row, [&](const Move &move) {
                CAPTURE(move);
                auto [colFrom, rowFrom] = move.colRowFromPosition();
                REQUIRE(colFrom == col);
                REQUIRE(rowFrom == row);
                REQUIRE(move.toPosition != move.fromPosition);
                REQUIRE(move.flag == Move::Flag::None);
                REQUIRE(destinations.find(move.toPosition) != destinations.end());
                calls++;
            });

            REQUIRE(calls == count);
        };

        SECTION("Board with single pawn has just one move") {
            board.setPiece(4, 4, Piece(Piece::Type::Pawn, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, 4, 4, 1);
        }

        SECTION("Board with single rook always has 14 moves") {
            uint8_t col = GENERATE(TEST_SOME(range(0u, 8u)));
            uint8_t row = GENERATE(TEST_SOME(range(0u, 8u)));
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
        [[maybe_unused]] Color other = opposite(Color::White);// TODO use or remove

        SECTION("Board with multiple pawns has multiple moves") {
            uint8_t count = GENERATE(range(2u, 8u));
            for (uint8_t i = 0; i < count; i++) {
                board.setPiece(i, 4, Piece(Piece::Type::Pawn, toMove));
            }
            MoveList list = generateAllMoves(board);
            REQUIRE(list.size() == count);
        }

        SECTION("Column containing pawns with nothing or another pawn above generates no moves") {
            uint8_t col = GENERATE(7u, TEST_SOME(range(0u, 7u)));
            uint8_t count = GENERATE(7u, TEST_SOME(range(0u, 7u)));

            for (uint8_t i = 0; i < count; i++) {
                uint8_t row = toMove == Chess::Color::White ? 7 - i : i;
                board.setPiece(col, row, Piece(Piece::Type::Pawn, toMove));
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
                list.forEachMoveFrom(i, i, [&](const Move &move) {
                    count++;
                    auto [colFrom, rowFrom] = move.colRowFromPosition();
                    auto [colTo, rowTo] = move.colRowToPosition();

                    REQUIRE((colTo == colFrom || rowTo == rowFrom));
                    REQUIRE(move.fromPosition != move.toPosition);
                    REQUIRE(move.flag == Move::Flag::None);
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
        [[maybe_unused]] Color other = opposite(Color::White);// TODO use or remove

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
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
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
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
                count++;
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
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
            Piece p{GENERATE(CAPTURABLE_TYPES), other};
            for (uint8_t col = 3; col <= 5; col++) {
                for (uint8_t row = 3; row <= 5; row++) {
                    board.setPiece(col, row, p);
                }
            }

            board.setPiece(4, 4, Piece{GENERATE(Piece::Type::Queen, Piece::Type::King), toMove});

            MoveList list = generateAllMoves(board);
            std::set<uint8_t> captures;
            list.forEachMoveFrom(4, 4, [&](const Move &move) {
                REQUIRE(move.toPosition != move.fromPosition);
                captures.insert(move.toPosition);
                auto [col, row] = move.colRowToPosition();
                CAPTURE(col, row);
                REQUIRE(board.pieceAt(col, row) == p);
            });
            REQUIRE(captures.size() == 8);
        }

        SECTION("Queen can capture pieces far away") {
            Piece p{GENERATE(CAPTURABLE_TYPES), other};
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
            std::set<uint8_t> captures;
            list.forEachMoveFrom(4, 4, [&](const Move &move) {
                REQUIRE(move.toPosition != move.fromPosition);
                auto [col, row] = move.colRowToPosition();
                CAPTURE(col, row);
                auto optPiece = board.pieceAt(col, row);
                if (optPiece) {
                    captures.insert(move.toPosition);
                    REQUIRE(optPiece == p);
                }
            });
            REQUIRE(captures.size() == 8);
        }

        SECTION("Knights can jump over pieces and capture") {
            Piece capturable{GENERATE(CAPTURABLE_TYPES), other};
            Piece blocker{GENERATE(ANY_TYPE), toMove};
            for (uint8_t col = 2; col <= 6; col++) {
                for (uint8_t row = 2; row <= 6; row++) {
                    if (col == 2 || col == 6 || row == 2 || row == 6) {
                        board.setPiece(col, row, capturable);
                    } else {
                        board.setPiece(col, row, blocker);
                    }
                }
            }

            board.setPiece(4, 4, Piece{Piece::Type::Knight, toMove});

            MoveList list = generateAllMoves(board);
            std::set<uint8_t> captures;
            list.forEachMoveFrom(4, 4, [&](const Move &move) {
                REQUIRE(move.toPosition != move.fromPosition);
                auto [col, row] = move.colRowToPosition();
                CAPTURE(col, row);
                REQUIRE(board.pieceAt(col, row) == capturable);
                captures.insert(move.toPosition);
            });
            REQUIRE(captures.size() == 8);
        }

        SECTION("Queen cannot capture pieces if blocked by own color") {
            // fill board
            Piece capturable{GENERATE(CAPTURABLE_TYPES), other};
            Piece blocker{GENERATE(ANY_TYPE), toMove};
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
            list.forEachMoveFrom(4, 4, [&](const Move &move) {
                REQUIRE(move.toPosition != move.fromPosition);
                auto [col, row] = move.colRowToPosition();
                CAPTURE(col, row);
                REQUIRE(board.pieceAt(col, row) == std::nullopt);
            });
        }
    }
}

TEST_CASE("Pawn move generation", "[chess][rules][movegen]") {
    using namespace Chess;

    Board board = Board::emptyBoard();
    if (GENERATE(true, false)) {
        board.makeNullMove();
    }
    Color toMove = board.colorToMove();
    Color other = opposite(toMove);

    CAPTURE(toMove);

    int8_t offset = toMove == Color::White ? 1 : -1;
    int8_t startRow = toMove == Color::White ? 1 : 6;
    int8_t endRow = toMove == Color::Black ? 1 : 6; // reverse of start

    SECTION("Pawns cannot capture anywhere (except direct diagonals)") {
        Piece capturable{GENERATE(CAPTURABLE_TYPES), other};
        for (uint8_t col = 0; col < 8; col++) {
            for (uint8_t row = 0; row < 8; row++) {
                if ((col != 3 && col != 5) || (row != 4 + offset)) {
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
        Piece capturable{GENERATE(CAPTURABLE_TYPES), other};
        unsigned expectedCaptures = 1;
        if (GENERATE(true, false)) {
            expectedCaptures = 2;
            board.setPiece(3, 4 + offset, capturable);
            board.setPiece(5, 4 + offset, capturable);
        } else {
            uint8_t col = GENERATE(3, 5);
            board.setPiece(col, 4 + offset, capturable);
        }

        bool blocked = GENERATE(true, false);
        CAPTURE(blocked);
        if (blocked) {
            Color c = GENERATE(Color::White, Color::Black);
            Piece moveBlocker{GENERATE(ANY_TYPE), c};
            board.setPiece(4, 4 + offset, moveBlocker);
        }

        board.setPiece(4, 4, Piece{Piece::Type::Pawn, toMove});

        MoveList list = generateAllMoves(board);
        std::set<uint8_t> captures;
        list.forEachMoveFrom(4, 4, [&](const Move& move) {
          REQUIRE(move.toPosition != move.fromPosition);
          auto [col, row] = move.colRowToPosition();
          CAPTURE(col, row);
          auto optPiece = board.pieceAt(col, row);
          if (optPiece) {
              captures.insert(move.toPosition);
              REQUIRE(optPiece == capturable);
          }
        });
        REQUIRE(captures.size() == expectedCaptures);
    }

    SECTION("Double push") {
        uint8_t col = GENERATE(TEST_SOME(range(0, 7)));
        CAPTURE(col, startRow);
        board.setPiece(col, startRow, Piece{Piece::Type::Pawn, toMove});

        MoveList list = generateAllMoves(board);
        std::set<uint8_t> destinationRows;
        list.forEachMoveFrom(col, startRow, [&](const Move& move) {
          REQUIRE(move.toPosition != move.fromPosition);
          auto [col2, row] = move.colRowToPosition();
          REQUIRE(col == col2);
          destinationRows.insert(row);
          if (row == startRow + offset + offset) {
              REQUIRE(move.flag == Move::Flag::DoublePushPawn);
          }
        });
        REQUIRE(destinationRows.size() == 2);
        REQUIRE(destinationRows.find(startRow + offset) != destinationRows.end());
        REQUIRE(destinationRows.find(startRow + offset + offset) != destinationRows.end());
    }

    SECTION("Double push is blocked by any piece at final position but single move is still possible") {
        uint8_t col = GENERATE(TEST_SOME(range(0, 7)));
        CAPTURE(col, startRow);
        board.setPiece(col, startRow, Piece{Piece::Type::Pawn, toMove});

        Piece moveBlocker{GENERATE(ANY_TYPE), GENERATE(Color::White, Color::Black)};
        board.setPiece(col, startRow + offset + offset, moveBlocker);

        MoveList list = generateAllMoves(board);
        std::set<uint8_t> destinationRows;
        list.forEachMoveFrom(col, startRow, [&](const Move& move) {
          REQUIRE(move.toPosition != move.fromPosition);
          auto [col2, row] = move.colRowToPosition();
          REQUIRE(col == col2);
          destinationRows.insert(row);
        });
        REQUIRE(destinationRows.size() == 1);
        REQUIRE(destinationRows.find(startRow + offset) != destinationRows.end());
        REQUIRE(destinationRows.find(startRow + offset + offset) == destinationRows.end());
    }

    SECTION("Piece directly in front of pawn stops both single and double pushes") {
        uint8_t col = GENERATE(0, 7);
        CAPTURE(col, startRow);
        board.setPiece(col, startRow, Piece{Piece::Type::Pawn, toMove});

        Piece moveBlocker{GENERATE(ANY_TYPE), GENERATE(Color::White, Color::Black)};
        board.setPiece(col, startRow + offset, moveBlocker);

        MoveList list = generateAllMoves(board);
        list.forEachMoveFrom(col, startRow, [&](const Move& move) {
          REQUIRE(false);
        });
    }

    SECTION("Pawns not on the starting row never give a double push") {
        uint8_t col = GENERATE(range(0u, 8u));
        uint8_t row = GENERATE_COPY(filter([startRow](uint8_t v){ return v != startRow; },range(0u, 8u)));
        board.setPiece(col, row, Piece(Piece::Type::Pawn, toMove));
        MoveList list = generateAllMoves(board);
        list.forEachMove([](const Move& move) {
          REQUIRE(move.flag != Move::Flag::DoublePushPawn);
        });
    }

    SECTION("Promotion") {
        uint8_t col = GENERATE(0, 7);
        CAPTURE(col, endRow);
        board.setPiece(col, endRow, Piece{Piece::Type::Pawn, toMove});

        MoveList list = generateAllMoves(board);
        std::set<Piece::Type> types;
        list.forEachMoveFrom(col, endRow, [&](const Move& move) {
          REQUIRE(move.toPosition != move.fromPosition);
          auto [col2, row] = move.colRowToPosition();
          REQUIRE(col == col2);
          REQUIRE(row == endRow + offset);
          REQUIRE(move.isPromotion());
          auto type = move.promotedType();
          REQUIRE(types.find(type) == types.end());
          types.insert(type);
        });
        REQUIRE(types.size() == 4);
    }

    SECTION("Capture promotion") {
        uint8_t col = GENERATE(0, 7);
        uint8_t captureCol = GENERATE_REF(filter([&](uint8_t i){return i >= 0 && i < board.size();}, values({col - 1, col + 1})));
        CAPTURE(col, endRow);

        board.setPiece(col, endRow, Piece{Piece::Type::Pawn, toMove});

        board.setPiece(captureCol, endRow + offset, Piece{GENERATE(CAPTURABLE_TYPES), other});

        bool hasBlocker = GENERATE(true, false);
        if (hasBlocker) {
            Piece moveBlocker{GENERATE(ANY_TYPE), GENERATE(Color::White, Color::Black)};
            board.setPiece(col, endRow + offset, moveBlocker);
        }

        MoveList list = generateAllMoves(board);
        std::set<Piece::Type> types;
        unsigned calls = 0;
        list.forEachMoveFrom(col, endRow, [&](const Move& move) {
          REQUIRE(move.toPosition != move.fromPosition);
          auto [col2, row] = move.colRowToPosition();;
          REQUIRE(row == endRow + offset);
          REQUIRE(move.isPromotion());
          auto pieceAt = board.pieceAt(col2, row);
          if (pieceAt != std::nullopt) {
              auto type = move.promotedType();
              REQUIRE(types.find(type) == types.end());
              types.insert(type);

              REQUIRE(col2 != col);
          }
          calls++;
        });
        REQUIRE(types.size() == 4);
        REQUIRE(calls == (hasBlocker ? 4 : 8));
    }

    SECTION("En passant") {
        uint8_t enPassantRowOther = endRow - offset;
        uint8_t rowAfterDoublePushOther = enPassantRowOther - offset;
        CAPTURE(enPassantRowOther, rowAfterDoublePushOther);

        uint8_t col = GENERATE(TEST_SOME(range(0u, 8u)));
        CAPTURE(col);
        {
            // setup en passant square via FEN (we do not want a method to set this)
            board.setPiece(col, rowAfterDoublePushOther, Piece{Piece::Type::Pawn, other});
            std::string baseFEN = board.toFEN();
            REQUIRE(baseFEN.ends_with(" - 0 1"));
            auto loc = baseFEN.rfind("- ");
            REQUIRE(loc != baseFEN.size());
            baseFEN.replace(loc, 1, Board::columnRowToSAN(col, enPassantRowOther));
            REQUIRE_FALSE(baseFEN.ends_with(" - 0 1"));
            auto eBoard = Board::fromFEN(baseFEN);
            REQUIRE(eBoard);
            board = eBoard.extract();
            REQUIRE(board.colorToMove() == toMove);
            REQUIRE(board.enPassantColRow() == std::make_pair(col, enPassantRowOther));
        }
        int8_t pawnOffset = GENERATE_COPY(filter([col](int8_t i){ return col + i >= 0 && col + i < 8; }, values({1, -1})));
        uint8_t myCol = col + pawnOffset;
        CAPTURE(board.toFEN(), myCol);


        SECTION("Pawn can take other pawn with en passant") {
            board.setPiece(myCol, rowAfterDoublePushOther, Piece{Piece::Type::Pawn, toMove});

            bool hasBlocker = GENERATE(true, false);
            if (hasBlocker) {
                Piece moveBlocker{GENERATE(ANY_TYPE), GENERATE(Color::White, Color::Black)};
                board.setPiece(myCol, rowAfterDoublePushOther + offset, moveBlocker);
            }
            CAPTURE(hasBlocker);
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(myCol, rowAfterDoublePushOther, [&](const Move& move) {
              REQUIRE(move.toPosition != move.fromPosition);
              auto [col2, row] = move.colRowToPosition();;

              auto pieceAt = board.pieceAt(col2, row);
              REQUIRE_FALSE(pieceAt.has_value());
              if (move.flag != Move::Flag::None) {
                  REQUIRE(col2 != myCol);
                  REQUIRE(col2 == col);
                  REQUIRE(row == enPassantRowOther);
                  REQUIRE(move.flag == Move::Flag::EnPassant);
              } else {
                  REQUIRE_FALSE(hasBlocker);
                  REQUIRE(move.flag == Move::Flag::None);
              }
              calls++;
            });
            REQUIRE(calls == (hasBlocker ? 1 : 2));
        }

        SECTION("Can only capture on the en passant square not the original position") {
            board.setPiece(myCol, enPassantRowOther, Piece{Piece::Type::Pawn, toMove});

            // always block
            Piece moveBlocker{Piece::Type::Bishop, GENERATE(Color::White, Color::Black)};
            board.setPiece(myCol, enPassantRowOther + offset, moveBlocker);
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(myCol, enPassantRowOther, [&](const Move& move) {
              REQUIRE(false);
            });
            REQUIRE(calls == 0);
        }

        SECTION("Can still capture the piece in the actual position") {
            board.setPiece(myCol, rowAfterDoublePushOther - offset, Piece{Piece::Type::Pawn, toMove});

            // always block
            Piece moveBlocker{Piece::Type::Rook, GENERATE(Color::White, Color::Black)};
            board.setPiece(myCol, rowAfterDoublePushOther, moveBlocker);
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(myCol, rowAfterDoublePushOther - offset, [&](const Move& move) {
              REQUIRE(move.fromPosition != move.toPosition);
              REQUIRE(move.flag == Move::Flag::None);
              auto [col2, row] = move.colRowToPosition();;
              auto pieceAt = board.pieceAt(col2, row);
              REQUIRE(pieceAt);
              REQUIRE(pieceAt->type() == Piece::Type::Pawn);
              REQUIRE(col2 == col);
              REQUIRE(row == rowAfterDoublePushOther);
              calls++;
            });
            REQUIRE(calls == 1);
        }

        SECTION("Cannot take en passant with non-pawn") {

        }
    }
}

TEST_CASE("Castling move generation", "[chess][rules][movegen]") {
    // TODO: oh oh gonna be hard!
    SECTION("Can castle") {

    }

    SECTION("Castling can be blocked") {

        SECTION("Piece between rook and king") {

        }

        SECTION("King is in check") {

        }

        SECTION("King would move through check") {

        }

        SECTION("King would end in check") {

        }
    }
}

TEST_CASE("In check/check move generation", "[chess][rules][movegen]") {
    // TODO: oh oh gonna be hard!

    SECTION("In check") {
        // TODO: oh oh gonna be hard!
        SECTION("Move the king away from being attacked") {

        }

        SECTION("Move piece in front of attack") {
            SECTION("Cannot move pinned piece in front") {

            }
        }

        SECTION("Capture direct attacker") {

            SECTION("Cannot capture with pinned piece") {

            }
        }

        SECTION("Single move escapes") {
            SECTION("Double push to escape check") {

            }
            SECTION("En passant to escape check") {

            }
            SECTION("Promotion to escape check") {

            }
            SECTION("Promotion capture to escape check") {

            }
        }

        SECTION("Cannot move pinned piece (even when not in check)") {

        }
    }
}

