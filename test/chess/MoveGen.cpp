#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/MoveGen.h>
#include <set>

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

#define REQUIRE_EMPTY(list)             \
    REQUIRE(list.size() == 0);          \
    list.forEachMove([](const auto &) { \
        REQUIRE(false);                 \
    })

#ifdef EXTENDED_TESTS
#define TRUE_FALSE() GENERATE(true, false)
#else
// compile time random to still check both colors (hopefully)
#define TRUE_FALSE() bool(((__TIME__[7] - '0') + __LINE__ + __COUNTER__) % 2)
#endif

#define CHECK_BOTH_COLORS() \
    if (TRUE_FALSE()) board.makeNullMove(); \

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
        CHECK_BOTH_COLORS()
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
        CHECK_BOTH_COLORS()
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
            uint8_t col = GENERATE(range(3, 5));
            uint8_t row = GENERATE_COPY(filter([=](uint8_t r){ return r != 4 || col != 4;}, range(3, 5)));
            board.setPiece(col, row, p);

            board.setPiece(4, 4, Piece{Piece::Type::Queen, toMove});

            CAPTURE(board.toFEN());
            MoveList list = generateAllMoves(board);
            unsigned captures = 0;
            list.forEachMoveFrom(4, 4, [&](const Move &move) {
              REQUIRE(move.toPosition != move.fromPosition);
              auto [col, row] = move.colRowToPosition();
              auto piece = board.pieceAt(col, row);
              if (!piece) {
                  return;
              }
              captures++;
              REQUIRE(board.pieceAt(col, row) == p);
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
    using namespace Chess;

    Board board = Board::emptyBoard();
    CHECK_BOTH_COLORS()
    Color toMove = board.colorToMove();
    Color other = opposite(toMove);

    CAPTURE(toMove);

    int8_t offset = toMove == Color::White ? 1 : -1;
    int8_t startRow = toMove == Color::White ? 1 : 6;
    int8_t endRow = toMove == Color::Black ? 1 : 6;// reverse of start

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
        list.forEachMoveFrom(col, startRow, [&](const Move &move) {
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
        uint8_t captureCol = GENERATE_REF(filter([&](uint8_t i) { return i >= 0 && i < board.size(); }, values({col - 1, col + 1})));
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
        int8_t pawnOffset = GENERATE_COPY(filter([col](int8_t i) { return col + i >= 0 && col + i < 8; }, values({1, -1})));
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
            list.forEachMoveFrom(myCol, enPassantRowOther, [&](const Move &move) {
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
}

TEST_CASE("Castling move generation", "[chess][rules][movegen]") {
    using namespace Chess;

    Board board = Board::emptyBoard();
    CHECK_BOTH_COLORS()
    Color toMove = board.colorToMove();
    Color other = opposite(toMove);

    uint8_t homeRow = toMove == Color::White ? 0 : 7;
    const uint8_t queenSideRook = 0;
    const uint8_t kingSideRook = 7;
    const uint8_t kingCol = 4;

    SECTION("Verify positions") {
        Board startPosition = Board::standardBoard();
        REQUIRE(startPosition.pieceAt(kingCol, homeRow) == Piece{Piece::Type::King, toMove});
        REQUIRE(startPosition.pieceAt(kingSideRook, homeRow) == Piece{Piece::Type::Rook, toMove});
        REQUIRE(startPosition.pieceAt(queenSideRook, homeRow) == Piece{Piece::Type::Rook, toMove});
    }

    bool kingSide = GENERATE(true, false);
    bool queenSide = GENERATE_COPY(filter([=](bool b) { return b || kingSide; }, values({0, 1})));

    bool withOppositeRook =
#ifdef EXTENDED_TESTS
            GENERATE_COPY(filter([=](bool b) { return b || (!queenSide || !kingSide); }, values({0, 1})));
#else
      false;
#endif

    CAPTURE(toMove, kingSide, queenSide, withOppositeRook);

    {
        // setup en passant square via FEN (we do not want a method to set this)
        board.setPiece(kingCol, homeRow, Piece{Piece::Type::King, toMove});

        std::string castles;

        if (kingSide || withOppositeRook) {
            board.setPiece(kingSideRook, homeRow, Piece{Piece::Type::Rook, toMove});
            if (kingSide) {
                castles += Piece{Piece::Type::King, toMove}.toFEN();
            }
        }

        if (queenSide || withOppositeRook) {
            board.setPiece(queenSideRook, homeRow, Piece{Piece::Type::Rook, toMove});
            if (queenSide) {
                castles += Piece{Piece::Type::Queen, toMove}.toFEN();
            }
        }

        std::string baseFEN = board.toFEN();
        auto loc = baseFEN.rfind("- - ");
        baseFEN.replace(loc, 1, castles);
        auto eBoard = Board::fromFEN(baseFEN);
        if (!eBoard) {
            INFO("Error - " << eBoard.error());
            REQUIRE(eBoard);
        }
        board = eBoard.extract();
        REQUIRE((board.castlingRights() & CastlingRight::AnyCastling) != CastlingRight::NoCastling);
        if (toMove == Color::White) {
            REQUIRE((board.castlingRights() & CastlingRight::WhiteCastling) != CastlingRight::NoCastling);
        } else {
            REQUIRE((board.castlingRights() & CastlingRight::BlackCastling) != CastlingRight::NoCastling);
        }
    }

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

            uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
            uint8_t row = GENERATE_COPY(filter([=](uint8_t i) { return i != homeRow; }, TEST_SOME(range(0, 8))));
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

    SECTION("King is in check") {
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
    using namespace Chess;

    Board board = Board::emptyBoard();
    CHECK_BOTH_COLORS()
    Color toMove = board.colorToMove();
    Color other = opposite(toMove);
    CAPTURE(toMove);

    Piece king{Piece::Type::King, toMove};

    // TODO: oh oh gonna be hard!

    SECTION("In check") {
        // TODO: oh oh gonna be hard!
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
        }

        SECTION("Capture attacker with not king") {
            board.setPiece(0, 0, king);
            board.setPiece(0, 7, Piece{Piece::Type::Rook, other});
            board.setPiece(1, 7, Piece{Piece::Type::Rook, toMove});

            board.setPiece(7, 7, Piece{Piece::Type::Pawn, toMove}); // this cannot move!
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
        }

        SECTION("Attacked by knight") {
            board.setPiece(0, 0, king);
            bool knightPos = GENERATE(true, false);
            if (knightPos) {
                board.setPiece(1, 2, Piece{Piece::Type::Knight, other});
            } else {
                board.setPiece(2, 1, Piece{Piece::Type::Knight, other});
            }

            board.setPiece(0, 7, Piece{Piece::Type::Pawn, toMove}); // this cannot move!
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
        }

        SECTION("Attacked by bishop") {
            board.setPiece(0, 0, king);
            uint8_t coord = GENERATE(TEST_SOME(range(2, 8)));
            board.setPiece(coord, coord, Piece{Piece::Type::Bishop, other});

            board.setPiece(0, 7, Piece{Piece::Type::Pawn, toMove}); // this cannot move!
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
        }

        SECTION("Block bishop or take it") {
            board.setPiece(0, 0, king);
            uint8_t coord = GENERATE(TEST_SOME(range(2, 8)));
            board.setPiece(coord, coord, Piece{Piece::Type::Bishop, other});
            board.setPiece(coord, 1, Piece{Piece::Type::Queen, toMove});

            board.setPiece(0, 7, Piece{Piece::Type::Pawn, toMove}); // this cannot move!

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
            CHECK(calls == 2 + (coord % 2)); // on the diagonal we can actually block twice
            REQUIRE(list.size() == calls + 2);
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


            board.setPiece(7, 4, Piece{Piece::Type::Pawn, toMove}); // this cannot move!
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
        }

        SECTION("Pawn is not bishop") {
            if (toMove == Color::White) {
                board.setPiece(0, 0, king);
                board.setPiece(2, 2, Piece{Piece::Type::Pawn, other});
            } else {
                board.setPiece(7, 7, king);
                board.setPiece(5, 5, Piece{Piece::Type::Pawn, other});
            }

            board.setPiece(7, 4, Piece{Piece::Type::Pawn, toMove}); // this cannot move!
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
            list.forEachMoveFrom(7, 4, [&](const Move& move) {
                otherCalls++;
            });
            CHECK(calls == 2);
            CHECK(otherCalls == 1);
            REQUIRE(list.size() == 3);
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
              REQUIRE(col2 == 0); // can only move up
              calls++;
            });
            REQUIRE(calls == 1);
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
              REQUIRE(col2 == 1); // can only move up
              calls++;
            });
            REQUIRE(calls == 2);
            list.forEachMoveFrom(0, 1, [&](const Move& move) {
               REQUIRE(false);
            });
            list.forEachMoveFrom(7, 4, [&](const Move& move) {
              REQUIRE(false);
            });
            REQUIRE(list.size() == 2);
        }

        SECTION("Cannot move pinned piece (even when not in check)") {
            board.setPiece(0, 0, king);
            board.setPiece(0, 1, Piece{Piece::Type::Bishop, toMove});
            board.setPiece(0, 7, Piece{Piece::Type::Rook, other});
            CAPTURE(board.toFEN());

            MoveList list = generateAllMoves(board);
            list.forEachMoveFrom(0, 1, [&](const Move &move) {
                REQUIRE(false);
            });
            REQUIRE(list.size() > 0);
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
        }
    }
}

TEST_CASE("Specific examples", "[chess][movegen]") {
    using namespace Chess;
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
    }
}