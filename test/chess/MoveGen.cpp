#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/MoveGen.h>
#include <set>

using namespace Chess;

TEST_CASE("Basic move checks", "[chess][move]") {
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

#define REQUIRE_EMPTY(list)             \
    REQUIRE(list.size() == 0);          \
    list.forEachMove([](const auto &) { \
        REQUIRE(false);                 \
    })

#define MOVES_NOT_CHECK_OR_STALEMATE() \
    REQUIRE_FALSE(list.isStaleMate()); \
    REQUIRE_FALSE(list.isCheckMate())

#define CHECK_BOTH_COLORS() \
    if (TRUE_FALSE()) board.makeNullMove();

TEST_CASE("Move generation basic", "[chess][rules][movegen]") {
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
        CHECK_BOTH_COLORS()
        Color toMove = board.colorToMove();

        auto validateCountAndFrom = [](const MoveList &list, BoardIndex col, BoardIndex row, unsigned count) {
            REQUIRE(list.size() == count);
            if (count > 0) {
                MOVES_NOT_CHECK_OR_STALEMATE();
            }
            std::set<BoardIndex> destinations;

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

        SECTION("King not on the edge has 8 moves") {
            uint8_t col = GENERATE(TEST_SOME(range(1, 7)));
            uint8_t row = GENERATE(TEST_SOME(range(1, 7)));
            board.setPiece(col, row, Piece(Piece::Type::King, toMove));
            MoveList list = generateAllMoves(board);
            validateCountAndFrom(list, col, row, 8);
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
        CHECK_BOTH_COLORS()
        Color toMove = board.colorToMove();

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
            for (uint8_t i = 0; i < Board::size; i++) {
                board.setPiece(i, i, Piece(Piece::Type::Rook, toMove));
            }
            MoveList list = generateAllMoves(board);
            REQUIRE(list.size() == 14 * Board::size);
            for (uint8_t i = 0; i < Board::size; i++) {
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
        CHECK_BOTH_COLORS()
        Color toMove = board.colorToMove();

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
            list.forEachMoveFrom(0, 0, [&](const Move&) {
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
        CHECK_BOTH_COLORS()
        Color toMove = board.colorToMove();
        Color other = opposite(toMove);

        CAPTURE(toMove);

        SECTION("Queen can capture all pieces around it") {
            Piece p{GENERATE(CAPTURABLE_TYPES), other};
            for (uint8_t col = 3; col <= 5; col++) {
                for (uint8_t row = 3; row <= 5; row++) {
                    board.setPiece(col, row, p);
                }
            }

            board.setPiece(4, 4, Piece{Piece::Type::Queen, toMove});

            CAPTURE(board.toFEN());
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

        SECTION("King can capture on all spots around") {
            Piece p{GENERATE(CAPTURABLE_TYPES), other};
            BoardIndex col = GENERATE(TEST_SOME(range(3, 5)));
            BoardIndex row = GENERATE_COPY(TEST_SOME(filter([=](uint8_t r) { return r != 4 || col != 4; }, range(3, 5))));
            board.setPiece(col, row, p);

            board.setPiece(4, 4, Piece{Piece::Type::King, toMove});

            CAPTURE(board.toFEN());
            MoveList list = generateAllMoves(board);
            unsigned captures = 0;
            list.forEachMoveFrom(4, 4, [&](const Move &move) {
                REQUIRE(move.toPosition != move.fromPosition);
                auto [colTo, rowTo] = move.colRowToPosition();
                auto piece = board.pieceAt(colTo, rowTo);
                if (!piece) {
                    return;
                }
                captures++;
                REQUIRE(board.pieceAt(colTo, rowTo) == p);
            });
            REQUIRE(captures == 1);
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

            CAPTURE(board.toFEN());
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
            Piece blocker{GENERATE(CAPTURABLE_TYPES), toMove};
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

            CAPTURE(board.toFEN());
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
    Board board = Board::emptyBoard();
    CHECK_BOTH_COLORS()
    Color toMove = board.colorToMove();
    Color other = opposite(toMove);

    CAPTURE(toMove);

    int8_t offset = Board::pawnDirection(toMove);
    int8_t startRow = Board::pawnHomeRow(toMove);
    int8_t endRow = Board::pawnHomeRow(opposite(toMove));// reverse of start

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

#define IS_BLOCKED() TRUE_FALSE()
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

        bool blocked = IS_BLOCKED();
        if (blocked) {
            Color c = GENERATE(Color::White, Color::Black);
            Piece moveBlocker{GENERATE(CAPTURABLE_TYPES), c};
            board.setPiece(4, 4 + offset, moveBlocker);
        }

        board.setPiece(4, 4, Piece{Piece::Type::Pawn, toMove});

        MoveList list = generateAllMoves(board);
        std::set<uint8_t> captures;
        list.forEachMoveFrom(4, 4, [&](const Move &move) {
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
        list.forEachMoveFrom(col, startRow, [&](const Move &move) {
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
        list.forEachMoveFrom(col, startRow, [&](const Move &move) {
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
        uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
        CAPTURE(col, startRow);
        board.setPiece(col, startRow, Piece{Piece::Type::Pawn, toMove});

        Piece moveBlocker{GENERATE(ANY_TYPE), GENERATE(Color::White, Color::Black)};
        board.setPiece(col, startRow + offset, moveBlocker);

        MoveList list = generateAllMoves(board);
        list.forEachMoveFrom(col, startRow, [&](const Move&) {
            REQUIRE(false);
        });
    }

    SECTION("Pawns not on the starting row never give a double push") {
        uint8_t col = GENERATE(TEST_SOME(range(0u, 8u)));
        uint8_t row = GENERATE_COPY(filter([startRow](uint8_t v) { return v != startRow; }, TEST_SOME(range(0u, 8u))));
        board.setPiece(col, row, Piece(Piece::Type::Pawn, toMove));
        MoveList list = generateAllMoves(board);
        list.forEachMove([](const Move &move) {
            REQUIRE(move.flag != Move::Flag::DoublePushPawn);
        });
    }

    SECTION("Promotion") {
        uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
        CAPTURE(col, endRow);
        board.setPiece(col, endRow, Piece{Piece::Type::Pawn, toMove});

        MoveList list = generateAllMoves(board);
        std::set<Piece::Type> types;
        list.forEachMoveFrom(col, endRow, [&](const Move &move) {
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
        uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
        uint8_t captureCol = GENERATE_REF(filter([&](uint8_t i) { return i >= 0 && i < Board::size; }, values({col - 1, col + 1})));
        CAPTURE(col, endRow);

        board.setPiece(col, endRow, Piece{Piece::Type::Pawn, toMove});

        board.setPiece(captureCol, endRow + offset, Piece{GENERATE(CAPTURABLE_TYPES), other});

        bool hasBlocker = IS_BLOCKED();
        if (hasBlocker) {
            Piece moveBlocker{GENERATE(ANY_TYPE), GENERATE(Color::White, Color::Black)};
            board.setPiece(col, endRow + offset, moveBlocker);
        }

        MoveList list = generateAllMoves(board);
        std::set<Piece::Type> types;
        unsigned calls = 0;
        list.forEachMoveFrom(col, endRow, [&](const Move &move) {
            REQUIRE(move.toPosition != move.fromPosition);
            auto [col2, row] = move.colRowToPosition();

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

        board = TestUtil::createEnPassantBoard(toMove, col);
        int8_t pawnOffset = GENERATE_COPY(filter([col](int8_t i) {
                return col + i >= 0 && col + i < 8;
            }, values({1, -1})));
        uint8_t myCol = col + pawnOffset;
        CAPTURE(myCol);


        SECTION("Pawn can take other pawn with en passant") {
            board.setPiece(myCol, rowAfterDoublePushOther, Piece{Piece::Type::Pawn, toMove});

            bool hasBlocker = IS_BLOCKED();
            if (hasBlocker) {
                Piece moveBlocker{GENERATE(CAPTURABLE_TYPES), GENERATE(Color::White, Color::Black)};
                board.setPiece(myCol, rowAfterDoublePushOther + offset, moveBlocker);
            }
            CAPTURE(hasBlocker);
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(myCol, rowAfterDoublePushOther, [&](const Move &move) {
                REQUIRE(move.toPosition != move.fromPosition);
                auto [col2, row] = move.colRowToPosition();

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
            list.forEachMoveFrom(myCol, enPassantRowOther, [&](const Move&) {
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
            list.forEachMoveFrom(myCol, rowAfterDoublePushOther - offset, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row] = move.colRowToPosition();
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
            board.setPiece(myCol, rowAfterDoublePushOther, Piece{Piece::Type::Bishop, toMove});

            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(myCol, rowAfterDoublePushOther, [&](const Move &move) {
                REQUIRE(move.toPosition != move.fromPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row] = move.colRowToPosition();

                auto pieceAt = board.pieceAt(col2, row);
                REQUIRE_FALSE(pieceAt.has_value());
            });
        }
    }
#undef IS_BLOCKED
}

TEST_CASE("Castling move generation", "[chess][rules][movegen]") {

    Color toMove = TRUE_FALSE() ? Color::White : Color::Black;
    Color other = opposite(toMove);

    const uint8_t homeRow = Board::homeRow(toMove);
    const uint8_t queenSideRook = Board::queenSideRookCol;
    const uint8_t kingSideRook = Board::kingSideRookCol;
    const uint8_t kingCol = Board::kingCol;

    bool kingSide = GENERATE(true, false);
    bool queenSide = GENERATE_COPY(filter([=](bool b) { return b || kingSide; }, values({1, 0})));

    bool withOppositeRook =
#ifdef EXTENDED_TESTS
            GENERATE_COPY(filter([=](bool b) { return b || (!queenSide || !kingSide); }, values({0, 1})));
#else
    false;
#endif
    bool withOppositeCastling = GENERATE(true, false);

    CAPTURE(toMove, kingSide, queenSide, withOppositeRook, withOppositeCastling);
    Board board = TestUtil::generateCastlingBoard(toMove, kingSide, queenSide, withOppositeRook, withOppositeCastling);

    SECTION("Can castle") {
        MoveList list = generateAllMoves(board);
        int calls = 0;
        list.forEachMoveFrom(kingCol, homeRow, [&](const Move &move) {
            REQUIRE(move.fromPosition != move.toPosition);
            auto [col2, row] = move.colRowToPosition();
            auto pieceAt = board.pieceAt(col2, row);
            if (!pieceAt.has_value()) {
                return;
            }
            REQUIRE(move.flag == Move::Flag::Castling);
            REQUIRE(pieceAt->type() == Piece::Type::Rook);
            REQUIRE((col2 == kingSideRook || col2 == queenSideRook));
            if (!kingSide) {
                REQUIRE(col2 == queenSideRook);
            }
            if (!queenSide) {
                REQUIRE(col2 == kingSideRook);
            }
            REQUIRE(row == homeRow);
            calls++;
        });
        REQUIRE(calls == kingSide + queenSide);
    }

    SECTION("Pieces do not block castling") {

        SECTION("Other king of same color on board") {
            // this is technically invalid though...
            board.setPiece(4, 4, Piece{Piece::Type::King, toMove});
        }

        SECTION("Any piece anywhere") {
            // just "our" color so no checks can occur
            Piece p{GENERATE(ANY_TYPE), toMove};

            // do not step on the toes of our opponent
            BoardIndex index = GENERATE(take(2,random(8u, 55u)));
            uint8_t col = index % 8u;
            uint8_t row = index / 8u;
            CHECK(row != 0);
            CHECK(row != 7);
            REQUIRE(board.pieceAt(col, row) == std::nullopt);
            board.setPiece(col, row, p);
        }

        if (queenSide) {
            SECTION("Queen side is not blocked by attack on b rank") {
                board.setPiece(1, 4, Piece{Piece::Type::Rook, other});
            }
        }

        int calls = 0;
        CAPTURE(board.toFEN());
        MoveList list = generateAllMoves(board);
        list.forEachMoveFrom(kingCol, homeRow, [&](const Move &move) {
            REQUIRE(move.fromPosition != move.toPosition);
            if (move.flag == Move::Flag::Castling) {
                auto [col2, row] = move.colRowToPosition();
                auto pieceAt = board.pieceAt(col2, row);
                REQUIRE(pieceAt.has_value());
                REQUIRE(pieceAt->type() == Piece::Type::Rook);
                REQUIRE((col2 == kingSideRook || col2 == queenSideRook));
                if (!kingSide) {
                    REQUIRE(col2 == queenSideRook);
                }
                if (!queenSide) {
                    REQUIRE(col2 == kingSideRook);
                }
                REQUIRE(row == homeRow);
                calls++;
            }
        });
        REQUIRE(calls == kingSide + queenSide);
    }

#define NO_CASTLING_MOVES()                                        \
    CAPTURE(board.toFEN());                                        \
    MoveList list = generateAllMoves(board);                       \
    list.forEachMoveFrom(kingCol, homeRow, [&](const Move &move) { \
        REQUIRE(move.fromPosition != move.toPosition);             \
        REQUIRE(move.flag != Move::Flag::Castling);                \
        auto [col2, row] = move.colRowToPosition();                \
        REQUIRE(col2 != kingSideRook);                             \
        REQUIRE(col2 != queenSideRook);                            \
    })

    SECTION("Castling is blocked by piece between rook and king") {

        Piece p{GENERATE(CAPTURABLE_TYPES), GENERATE(Color::White, Color::Black)};

        SECTION("Full homerow blocked") {
            for (uint8_t col = 0; col < 8; col++) {
                if (board.pieceAt(col, homeRow) == std::nullopt) {
                    board.setPiece(col, homeRow, p);
                }
            }
            NO_CASTLING_MOVES();
        }

        SECTION("Single piece") {
            if (kingSide) {
                uint8_t col = GENERATE_COPY(TEST_SOME(range(uint8_t(kingCol + 1), kingSideRook)));
                board.setPiece(col, homeRow, p);
            }
            if (queenSide) {
                uint8_t col = GENERATE_COPY(TEST_SOME(range(uint8_t(queenSideRook + 1), kingCol)));
                board.setPiece(col, homeRow, p);
            }
            NO_CASTLING_MOVES();
        }
    }

    SECTION("Cannot castle when king is in check") {
        board.setPiece(kingCol, 4, Piece{Piece::Type::Rook, other});
        NO_CASTLING_MOVES();
    }

    SECTION("King would move through/into check") {
        if (kingSide) {
            uint8_t col = GENERATE_COPY(TEST_SOME(range(uint8_t(kingCol + 1), kingSideRook)));
            board.setPiece(col, 4, Piece{Piece::Type::Rook, other});
        }
        if (queenSide) {
            // queenSideRook + 1 does not actually block castling queenside
            uint8_t col = GENERATE_COPY(TEST_SOME(range(uint8_t(queenSideRook + 2), kingCol)));
            board.setPiece(col, 4, Piece{Piece::Type::Rook, other});
        }
        NO_CASTLING_MOVES();
    }

#undef NO_CASTLING_MOVES
}

TEST_CASE("In check/check move generation", "[chess][rules][movegen]") {
    Board board = Board::emptyBoard();
    CHECK_BOTH_COLORS()
    Color toMove = board.colorToMove();
    Color other = opposite(toMove);
    CAPTURE(toMove);

    Piece king{Piece::Type::King, toMove};

    SECTION("In check") {
        SECTION("Move the king away from being attacked") {
            uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
            uint8_t row = GENERATE(TEST_SOME(range(0, 6)));
            board.setPiece(col, row, king);
            board.setPiece(col, 7, Piece{Piece::Type::Rook, other});
            CAPTURE(board.toFEN());
            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(3, 4, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                REQUIRE(col2 != col);
            });
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Move piece inbetween attack on king") {
            board.setPiece(0, 0, king);
            board.setPiece(7, 7, Piece{Piece::Type::King, other});
            board.setPiece(0, 7, Piece{Piece::Type::Rook, other});
            board.setPiece(1, 1, Piece{Piece::Type::Rook, toMove});
            CAPTURE(board.toFEN());
            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(1, 1, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                CHECK(col2 == 0);
                CHECK(row2 == 1);
                calls++;
            });
            CHECK(calls == 1);
            REQUIRE(list.size() == 2);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Capture attacker with not king") {
            board.setPiece(0, 0, king);
            board.setPiece(0, 7, Piece{Piece::Type::Rook, other});
            board.setPiece(1, 7, Piece{Piece::Type::Rook, toMove});

            board.setPiece(7, 6, Piece{Piece::Type::Pawn, toMove});// this cannot move!
            CAPTURE(board.toFEN());
            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(1, 7, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                CHECK(col2 == 0);
                CHECK(row2 == 7);
                REQUIRE(board.pieceAt(col2, row2) == Piece{Piece::Type::Rook, other});
                calls++;
            });
            CHECK(calls == 1);
            REQUIRE(list.size() == 3);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Attacked by knight king can run away") {
            board.setPiece(0, 0, king);
            bool knightPos = GENERATE(true, false);
            if (knightPos) {
                board.setPiece(1, 2, Piece{Piece::Type::Knight, other});
            } else {
                board.setPiece(2, 1, Piece{Piece::Type::Knight, other});
            }

            board.setPiece(0, 6, Piece{Piece::Type::Pawn, toMove});// this cannot move!
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                calls++;
            });
            CHECK(calls == 3);
            REQUIRE(list.size() == 3);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Attacked by knight then knight can be taken") {
            Piece defender{GENERATE(Piece::Type::Queen, Piece::Type::Pawn, Piece::Type::Bishop), toMove};

            board.setPiece(0, Board::homeRow(toMove), king);
            board.setPiece(1, Board::pawnHomeRow(toMove) + Board::pawnDirection(toMove), Piece{Piece::Type::Knight, other});

            board.setPiece(0, Board::pawnHomeRow(toMove), defender);
            CAPTURE(defender);

            board.setPiece(7, 4, Piece{Piece::Type::Pawn, toMove});// this cannot move!

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(0, Board::pawnHomeRow(toMove), [&](const Move &move) {
              REQUIRE(move.fromPosition != move.toPosition);
              REQUIRE(move.flag == Move::Flag::None);
              auto [colTo, rowTo] = move.colRowToPosition();
              auto piece = board.pieceAt(colTo, rowTo);
              REQUIRE(piece.has_value());
              CHECK(piece->type() == Piece::Type::Knight);
              calls++;
            });
            CHECK(calls == 1);
            REQUIRE(list.size() == 3);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Attacked by bishop") {
            board.setPiece(0, 0, king);
            uint8_t coord = GENERATE(TEST_SOME(range(2, 8)));
            board.setPiece(coord, coord, Piece{Piece::Type::Bishop, other});

            board.setPiece(0, 7, Piece{Piece::Type::Pawn, toMove});// this cannot move!
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                calls++;
            });
            CHECK(calls == 2);
            REQUIRE(list.size() == 2);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Block bishop or take it") {
            board.setPiece(0, 0, king);
            uint8_t coord = GENERATE(TEST_SOME(range(2, 8)));
            board.setPiece(coord, coord, Piece{Piece::Type::Bishop, other});
            board.setPiece(coord, 1, Piece{Piece::Type::Queen, toMove});

            board.setPiece(0, 7, Piece{Piece::Type::Pawn, toMove});// this cannot move!

            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            int calls = 0;
            list.forEachMoveFrom(coord, 1, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                REQUIRE(col2 == row2);
                auto p = board.pieceAt(col2, row2);
                if (col2 == coord) {
                    CHECK(p == Piece{Piece::Type::Bishop, other});
                } else {
                    CHECK_FALSE(p.has_value());
                }
                calls++;
            });
            CHECK(calls == 2 + (coord % 2));// on the diagonal we can actually block twice
            REQUIRE(list.size() == calls + 2u);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Pawn is also an attacker") {
            if (toMove == Color::White) {
                board.setPiece(0, 0, king);
                board.setPiece(1, 1, Piece{Piece::Type::Pawn, other});
                board.setPiece(2, 2, Piece{Piece::Type::Knight, other});
            } else {
                board.setPiece(7, 7, king);
                board.setPiece(6, 6, Piece{Piece::Type::Pawn, other});
                board.setPiece(5, 5, Piece{Piece::Type::Knight, other});
            }


            board.setPiece(7, 4, Piece{Piece::Type::Pawn, toMove});// this cannot move!
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            auto [kingCol, kingRow] = board.kingSquare(toMove);
            list.forEachMoveFrom(kingCol, kingRow, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                REQUIRE(col2 == row2);
                REQUIRE(board.pieceAt(col2, row2) == Piece{Piece::Type::Pawn, other});
                calls++;
            });
            CHECK(calls == 1);
            REQUIRE(list.size() == 1);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Pawn is not bishop") {
            if (toMove == Color::White) {
                board.setPiece(0, 0, king);
                board.setPiece(2, 2, Piece{Piece::Type::Pawn, other});
            } else {
                board.setPiece(7, 7, king);
                board.setPiece(5, 5, Piece{Piece::Type::Pawn, other});
            }

            board.setPiece(7, 4, Piece{Piece::Type::Pawn, toMove});// this cannot move!
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            auto [kingCol, kingRow] = board.kingSquare(toMove);
            list.forEachMoveFrom(kingCol, kingRow, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                REQUIRE(col2 != row2);
                REQUIRE_FALSE(board.pieceAt(col2, row2));
                calls++;
            });
            unsigned otherCalls = 0;
            list.forEachMoveFrom(7, 4, [&](const Move&) {
                otherCalls++;
            });
            CHECK(calls == 2);
            CHECK(otherCalls == 1);
            REQUIRE(list.size() == 3);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Cannot move next to other king") {
            board.setPiece(0, 0, king);
            board.setPiece(2, 0, Piece{Piece::Type::King, other});

            CAPTURE(board.toFEN());

            int calls = 0;
            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                REQUIRE(col2 == 0);// can only move up
                calls++;
            });
            REQUIRE(calls == 1);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Cannot take with pinned piece") {
            board.setPiece(0, 0, king);
            board.setPiece(1, 2, Piece{Piece::Type::Knight, other});

            board.setPiece(0, 1, Piece{Piece::Type::Bishop, toMove});
            board.setPiece(0, 7, Piece{Piece::Type::Rook, other});

            board.setPiece(7, 4, Piece{Piece::Type::Pawn, toMove});

            CAPTURE(board.toFEN());

            int calls = 0;
            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row2] = move.colRowToPosition();
                REQUIRE(col2 == 1);// can only move up
                calls++;
            });
            REQUIRE(calls == 2);
            list.forEachMoveFrom(0, 1, [&](const Move&) {
                REQUIRE(false);
            });
            list.forEachMoveFrom(7, 4, [&](const Move&) {
                REQUIRE(false);
            });
            REQUIRE(list.size() == 2);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Cannot move pinned piece (even when not in check)") {
            board.setPiece(0, 0, king);
            board.setPiece(0, 1, Piece{Piece::Type::Bishop, toMove});
            board.setPiece(0, 7, Piece{Piece::Type::Rook, other});
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(0, 1, [&](const Move&) {
                REQUIRE(false);
            });
            REQUIRE(list.size() > 0);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }

        SECTION("Cannot defend by jumping over pieces") {
            board.setPiece(0, 0, king);
            Piece blockingPiece = GENERATE_COPY(Piece{Piece::Type::Rook, other}, Piece{Piece::Type::Bishop, toMove});
            board.setPiece(2, 0, Piece{Piece::Type::Rook, other});
            board.setPiece(3, 0, blockingPiece);
            board.setPiece(4, 0, Piece{Piece::Type::Rook, toMove});
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(4, 0, [&](const Move&) {
                REQUIRE(false);
            });
            REQUIRE(list.size() > 0);
            MOVES_NOT_CHECK_OR_STALEMATE();
        }
    }

    SECTION("Cannot move into check") {
        SECTION("Do not move into rook") {
            board.setPiece(0, 0, king);
            board.setPiece(1, GENERATE(TEST_SOME(range(2, 8))), Piece{Piece::Type::Rook, other});
            CAPTURE(board.toFEN());
            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row] = move.colRowToPosition();
                REQUIRE(col2 == 0);
            });
            MOVES_NOT_CHECK_OR_STALEMATE();
        }
    }
}

TEST_CASE("Stale/check mate", "[chess][rules][movegen]") {
    Board board = Board::emptyBoard();
    CHECK_BOTH_COLORS()
    Color toMove = board.colorToMove();
    Color other = opposite(toMove);
    CAPTURE(toMove);

    Piece king{Piece::Type::King, toMove};
    Piece otherKing{Piece::Type::King, other};

    SECTION("If king in check and cannot move it is checkmate") {
        board.setPiece(0, 0, king);
        board.setPiece(0, 7, Piece{Piece::Type::Rook, other});
        board.setPiece(1, 7, Piece{Piece::Type::Rook, other});
        board.setPiece(7, 7, otherKing);

        MoveList list = generateAllMoves(board);
        REQUIRE(list.size() == 0);
        REQUIRE(list.isCheckMate());
        REQUIRE_FALSE(list.isStaleMate());
    }

    SECTION("If king not in check but cannot move it is stalemate") {
        board.setPiece(0, 0, king);
        board.setPiece(1, 7, Piece{Piece::Type::Rook, other});
        board.setPiece(7, 1, Piece{Piece::Type::Rook, other});
        board.setPiece(7, 7, otherKing);

        MoveList list = generateAllMoves(board);
        REQUIRE(list.size() == 0);
        REQUIRE_FALSE(list.isCheckMate());
        REQUIRE(list.isStaleMate());
    }

    SECTION("If king not in check but cannot move but has other moves it is not stalemate") {
        board.setPiece(0, 0, king);
        board.setPiece(1, 7, Piece{Piece::Type::Rook, other});
        board.setPiece(7, 1, Piece{Piece::Type::Rook, other});
        board.setPiece(7, 7, otherKing);

        board.setPiece(4, 4, Piece{Piece::Type::Pawn, toMove});

        MoveList list = generateAllMoves(board);
        REQUIRE(list.size() != 0);
        REQUIRE_FALSE(list.isCheckMate());
        REQUIRE_FALSE(list.isStaleMate());
    }

    SECTION("In checkmate having other moves does not matter") {
        board.setPiece(0, 0, king);
        board.setPiece(0, 7, Piece{Piece::Type::Rook, other});
        board.setPiece(1, 7, Piece{Piece::Type::Rook, other});
        board.setPiece(7, 7, otherKing);

        board.setPiece(4, 4, Piece{Piece::Type::Pawn, toMove});

        MoveList list = generateAllMoves(board);
        REQUIRE(list.size() == 0);
        REQUIRE(list.isCheckMate());
        REQUIRE_FALSE(list.isStaleMate());
    }
}

TEST_CASE("Specific examples", "[chess][movegen]") {
    SECTION("Move count") {

#define HAS_N_MOVES(fen, n)                      \
    do {                                         \
        INFO(fen);                               \
        auto b = Board::fromFEN(fen);            \
        if (!b) {                                \
            CAPTURE(b.error());                  \
            REQUIRE(b);                          \
        }                                        \
        Board board = b.extract();               \
        MoveList list = generateAllMoves(board); \
        REQUIRE(list.size() == (n));             \
    } while (false)

        SECTION("Examples from chessprogramming") {
            HAS_N_MOVES("3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1", 218);
            HAS_N_MOVES("R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1", 218);
            // only more because we separate promotions (4 possible promos)
            HAS_N_MOVES("1b1Q2b1/PQ4QP/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1", 224);
        }

        SECTION("Standard position") {
            HAS_N_MOVES("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 8 * 2 + 2 + 2);
        }

        SECTION("In check") {
            HAS_N_MOVES("2rr3k/8/8/8/7q/2K5/8/3b4 w - - 0 1", 1);
            HAS_N_MOVES("2rr3k/8/8/8/7q/2K5/8/8 w - - 0 1", 2);
            HAS_N_MOVES("2rr3k/8/8/8/8/2K5/8/8 w - - 0 1", 3);
            HAS_N_MOVES("3r3k/8/8/8/8/2K5/8/8 w - - 0 1", 5);
        }

        SECTION("Examples from github gist") {
            // https://gist.github.com/peterellisjones/8c46c28141c162d1d8a0f0badbc9cff9
            // https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/

            HAS_N_MOVES("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2", 8);
            HAS_N_MOVES("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 5 3", 8);
            HAS_N_MOVES("r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2", 19);
            HAS_N_MOVES("r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2", 5);
            HAS_N_MOVES("2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2", 44);
            HAS_N_MOVES("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9", 39);
            HAS_N_MOVES("2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4", 9);
        }

        SECTION("Single move no check") {
            HAS_N_MOVES("k5r1/3p4/8/2pP4/2P1p3/4P3/r7/7K w - - 0 1", 1);
        }

        SECTION("More positions from chessprogramming (really for perft)") {
            // from: https://www.chessprogramming.org/Perft_Results
            HAS_N_MOVES("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 48);

            HAS_N_MOVES("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 14);

            HAS_N_MOVES("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 6);
            //mirrored
            HAS_N_MOVES("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", 6);

            HAS_N_MOVES("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 44);

            // says depth 0 is 1 move??
            HAS_N_MOVES("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 46);
        }
        // More perft tests: https://github.com/elcabesa/vajolet/tree/develop/tests
    }

    SECTION("Checks") {
        SECTION("Cannot even move into rook which is pinned") {
            Board board = Board::fromFEN("8/8/8/8/8/Rrk5/8/K7 w - - 0 1").extract();
            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(0, 0, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                auto [col2, row] = move.colRowToPosition();
                REQUIRE(col2 == 0);
            });
        }

        SECTION("Cannot en passant with pinned pawn") {
            Board board = Board::fromFEN("7k/8/8/2KPp2r/8/8/8/8 w - e6 0 1").extract();
            MoveList list = generateAllMoves(board);
            unsigned calls = 0;
            list.forEachMoveFrom(3, 4, [&](const Move &move) {
                REQUIRE(move.fromPosition != move.toPosition);
                REQUIRE(move.flag == Move::Flag::None);
                calls++;
            });
            REQUIRE(calls == 1);
        }

        SECTION("Can take knight when being checked by it") {
            Board board = Board::fromFEN("rnbqkb1r/pppppppp/8/8/4n3/5P2/PPPPPKPP/RNBQ1BNR w kq - 3 3").extract();
            MoveList list = generateAllMoves(board);
            CHECK(list.size() == 3);
            unsigned calls = 0;
            list.forEachMoveFrom(5, 2, [&](const Move &move) {
              REQUIRE(move.fromPosition != move.toPosition);
              REQUIRE(move.flag == Move::Flag::None);
              calls++;
              auto [colTo, rowTo] = move.colRowToPosition();
              CHECK(colTo == 4);
              CHECK(rowTo == 3);
              auto piece = board.pieceAt(colTo, rowTo);
              REQUIRE(piece.has_value());
              CHECK(piece->type() == Piece::Type::Knight);
            });
            CHECK(calls == 1);
        }
    }

    auto fakeMove = [](Board &board, const Move &move) {
        auto [colFrom, rowFrom] = move.colRowFromPosition();
        auto [colTo, rowTo] = move.colRowToPosition();
        auto optPiece = board.pieceAt(colFrom, rowFrom);
        REQUIRE(optPiece.has_value());
        REQUIRE(optPiece->color() == board.colorToMove());
        board.setPiece(colTo, rowTo, optPiece.value());
        board.setPiece(colFrom, rowFrom, std::nullopt);

        board.makeNullMove();
    };

    SECTION("Single move possible") {

        SECTION("Sequence of 13 (!) consecutive forced moves") {
            Board board = Board::fromFEN("BQ4R1/2Q5/3Q4/4Q1pp/5B1P/6QK/Rrrrrrrq/R4nk1 w - - 0 1").extract();
            for (int i = 0; i < 13; i++) {
                MoveList list = generateAllMoves(board);
                REQUIRE(list.size() == 1);
                // fake make the move
                bool madeMove = false;
                list.forEachMove([&](const Move& move) {
                    REQUIRE_FALSE(madeMove);
                    fakeMove(board, move);
                    madeMove = true;
                });
                REQUIRE(madeMove);
            }
        }


        SECTION("Weird stale/check mate depending on side to move") {
            Board board = Board::fromFEN("qrb5/bk1p4/rp1P4/pPp2p1p/P1P2PpP/4p1PR/4P1KB/5BRQ w - - 0 1").extract();
            if (GENERATE(true, false)) {
                board.makeNullMove();
            }
            Color toMove = board.colorToMove();
            Color other = opposite(toMove);

            bool checkMate = GENERATE(true, false);
            CAPTURE(toMove, checkMate);

            {
                MoveList list = generateAllMoves(board);
                REQUIRE(list.size() == 1);
                MOVES_NOT_CHECK_OR_STALEMATE();
                bool madeMove = false;
                list.forEachMove([&](const Move& move) {
                  REQUIRE_FALSE(madeMove);
                  auto [colTo, rowTo] = move.colRowToPosition();
                  auto rook = board.pieceAt(colTo, rowTo);
                  REQUIRE(rook);
                  REQUIRE(rook->type() == Piece::Type::Rook);
                  REQUIRE(rook->color() == other);
                  fakeMove(board, move);
                  madeMove = true;
                });
                REQUIRE(madeMove);
            }

            {
                MoveList list = generateAllMoves(board);
                REQUIRE(list.size() == 2);
                MOVES_NOT_CHECK_OR_STALEMATE();
                Move toMake;
                bool foundMove = false;
                list.forEachMove([&](const Move& move) {
                  auto [colFrom, rowFrom] = move.colRowFromPosition();

                  auto king = board.pieceAt(colFrom, rowFrom);
                  REQUIRE(king);
                  REQUIRE(king->type() == Piece::Type::King);
                  REQUIRE(king->color() == other);

                  auto [colTo, rowTo] = move.colRowToPosition();
                  auto piece = board.pieceAt(colTo, rowTo);
                  if (piece) {
                      REQUIRE(piece->type() == Piece::Type::Pawn);
                      // actually a move from the opponent
                      REQUIRE(piece->color() == toMove);
                  }

                  if (piece.has_value() == checkMate) {
                      REQUIRE_FALSE(foundMove);
                      toMake = move;
                      foundMove = true;
                  }
                });
                REQUIRE(foundMove);
                fakeMove(board, toMake);
            }

            MoveList list = generateAllMoves(board);

            REQUIRE(list.isCheckMate() == checkMate);
            REQUIRE(list.isStaleMate() != checkMate);
            REQUIRE(list.size() == 0);
        }
    }

    SECTION("Every special move") {
        Board board = Board::fromFEN("5n2/6P1/8/k1pP4/pp6/8/7P/R3K2R w KQ c6 0 1").extract();
        MoveList list = generateAllMoves(board);
        std::multiset<Move::Flag> flags;
        list.forEachMove([&](const Move& move) {
            flags.insert(move.flag);
        });
        REQUIRE(flags.count(Move::Flag::EnPassant) == 1);
        REQUIRE(flags.count(Move::Flag::DoublePushPawn) == 1);
        REQUIRE(flags.count(Move::Flag::Castling) == 2);
        REQUIRE(flags.count(Move::Flag::PromotionToQueen) == 2);
        REQUIRE(flags.count(Move::Flag::PromotionToBishop) == 2);
        REQUIRE(flags.count(Move::Flag::PromotionToRook) == 2);
        REQUIRE(flags.count(Move::Flag::PromotionToKnight) == 2);
    }

    SECTION("Castling") {
        SECTION("Cannot castle just because the other side can") {
            Board board = Board::fromFEN("r3k2r/pbppqpb1/1n2p1p1/3PN3/1p2n3/2N2Q1p/PPPBBPPP/R3K2R w Kkq - 2 3").extract();
            MoveList list = generateAllMoves(board);
            unsigned count = 0;
            list.forEachMoveFrom(Board::kingCol, Board::homeRow(Color::White), [&](const Move& move) {
              if (move.flag == Move::Flag::Castling) {
                  count++;
                  auto [colTo, rowTo] = move.colRowToPosition();
                  CHECK(colTo == Board::kingSideRookCol);
              }
            });
            REQUIRE(count == 1);
        }
    }

}