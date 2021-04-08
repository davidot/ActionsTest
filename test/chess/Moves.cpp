#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/Board.h>

#define MAKE_VALID_MOVE(...) \
    {                     \
        auto currentCol = board.colorToMove(); \
        auto madeMove = board.makeMove(__VA_ARGS__);    \
        REQUIRE(madeMove);  \
        REQUIRE(board.colorToMove() == opposite(currentCol)); \
    }

TEST_CASE("Apply moves to board", "[chess][move]") {
    using namespace Chess;

    Board board = Board::emptyBoard();

    SECTION("No undo on empty board") {
        REQUIRE_FALSE(board.undoMove());
    }

    SECTION("Queen can move from anywhere to anywhere else") {
        uint8_t fromCol = GENERATE(TEST_SOME(range(0, 8)));
        uint8_t fromRow = GENERATE(TEST_SOME(range(0, 8)));

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

        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        const Piece piece = Piece{Piece::Type::Queen, c};
        board.setPiece(fromCol, fromRow, piece);

        MAKE_VALID_MOVE(Move{fromCol, fromRow, toCol, toRow});

        REQUIRE(board.pieceAt(toCol, toRow) == piece);
        REQUIRE_FALSE(board.pieceAt(fromCol, fromRow).has_value());

        SECTION("Undo move") {
            REQUIRE(board.undoMove());
            REQUIRE_FALSE(board.pieceAt(toCol, toRow).has_value());
            REQUIRE(board.pieceAt(fromCol, fromRow) == piece);
        }
    }

    SECTION("Can redo multiple moves") {
        uint8_t startCol = 4;
        uint8_t startRow = 4;

        std::vector<std::pair<uint8_t, uint8_t>> moves = {
                {4, 5},
                {2, 7},
                {2, 2},
                {0, 0},
                {5, 0},
                {7, 2},
                {2, 7},
                {2, 4},
                {4, 4},
        };

        int steps = GENERATE_COPY(range(2, (int)moves.size()));
        CAPTURE(steps);

        uint8_t currCol = startCol;
        uint8_t currRow = startRow;

        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        Piece piece = Piece{Piece::Type::Queen, c};
        board.setPiece(currCol, currRow, piece);

        // we just verify we can get back to the beginning for now
        for (int i = 0; i < steps; i++) {
            auto [stepCol, stepRow] = moves[i];
            Move mv{currCol, currRow, stepCol, stepRow};
            MAKE_VALID_MOVE(mv);

            REQUIRE_FALSE(board.pieceAt(currCol, currRow).has_value());
            REQUIRE(board.pieceAt(stepCol, stepRow) == piece);

            currCol = stepCol;
            currRow = stepRow;
            // get back to our color
            board.makeNullMove();
        }

        for (int i = 0; i < steps; i++) {
            auto [stepCol, stepRow] = moves[steps - 1 - i];
            REQUIRE(board.pieceAt(stepCol, stepRow) == piece);
            REQUIRE(board.countPieces(c) == 1);
            REQUIRE(board.undoMove());
        }

        REQUIRE(board.pieceAt(startCol, startRow) == piece);
        REQUIRE_FALSE(board.undoMove());
    }

    SECTION("Can move pawn in right direction") {
        uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
        uint8_t fromRow = GENERATE(TEST_SOME(range(2, 6)));


        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        uint8_t toRow = fromRow + Board::pawnDirection(c);

        const Piece piece = Piece{Piece::Type::Pawn, c};
        board.setPiece(col, fromRow, piece);

        MAKE_VALID_MOVE(Move{col, fromRow, col, toRow});

        REQUIRE(board.pieceAt(col, toRow) == piece);
        REQUIRE_FALSE(board.pieceAt(col, fromRow).has_value());

        SECTION("Undo move") {
            REQUIRE(board.undoMove());
            REQUIRE_FALSE(board.pieceAt(col, toRow).has_value());
            REQUIRE(board.pieceAt(col, fromRow) == piece);
        }
    }

#define CAPTURABLE_TYPES TEST_SOME(values({Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen}))

    SECTION("Can capture another piece") {
        uint8_t fromCol = 1;
        uint8_t fromRow = 1;

        uint8_t toCol = 1;
        uint8_t toRow = 2;

        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        const Piece piece = Piece{Piece::Type::Queen, c};
        const Piece capturing = Piece{GENERATE(CAPTURABLE_TYPES), opposite(c)};

        board.setPiece(fromCol, fromRow, piece);
        board.setPiece(toCol, toRow, capturing);

        MAKE_VALID_MOVE(Move{fromCol, fromRow, toCol, toRow});

        REQUIRE(board.pieceAt(toCol, toRow) == piece);
        REQUIRE_FALSE(board.pieceAt(fromCol, fromRow).has_value());

        SECTION("Undo places exact piece back") {
            REQUIRE(board.undoMove());
            REQUIRE(board.pieceAt(toCol, toRow) == capturing);
            REQUIRE(board.pieceAt(fromCol, fromRow) == piece);
        }
    }

    SECTION("Can perform promotion") {
        uint8_t col = GENERATE(TEST_SOME(range(0, 7)));

        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        uint8_t fromRow = Board::pawnHomeRow(opposite(c));
        uint8_t toRow = Board::homeRow(opposite(c));

        const Piece pawn = Piece{Piece::Type::Pawn, c};
        board.setPiece(col, fromRow, pawn);
        Move::Flag flag = GENERATE(Move::Flag::PromotionToQueen, Move::Flag::PromotionToKnight, Move::Flag::PromotionToBishop, Move::Flag::PromotionToRook);
        Move move = Move{col, fromRow, col, toRow, flag};

        MAKE_VALID_MOVE(move);


        REQUIRE_FALSE(board.pieceAt(col, fromRow).has_value());

        auto promotedPiece = board.pieceAt(col, toRow);
        REQUIRE(promotedPiece.has_value());
        REQUIRE(promotedPiece->type() == move.promotedType());

        SECTION("Undo puts back a pawn") {
            REQUIRE(board.undoMove());
            REQUIRE_FALSE(board.pieceAt(col, toRow).has_value());
            REQUIRE(board.pieceAt(col, fromRow) == pawn);
        }
    }

    SECTION("Double push sets en passant") {
        REQUIRE_FALSE(board.enPassantColRow().has_value());

        uint8_t col = GENERATE(TEST_SOME(range(0, 7)));

        Color c = GENERATE(Color::White, Color::Black);
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        CAPTURE(col, c);

        uint8_t fromRow = Board::pawnHomeRow(c);
        uint8_t toRow = fromRow + Board::pawnDirection(c) * 2;

        const Piece pawn = Piece{Piece::Type::Pawn, c};
        board.setPiece(col, fromRow, pawn);
        Move move = Move{col, fromRow, col, toRow, Move::Flag::DoublePushPawn};

        MAKE_VALID_MOVE(move);

        REQUIRE_FALSE(board.pieceAt(col, fromRow).has_value());
        REQUIRE(board.pieceAt(col, toRow) == pawn);

        auto enPassant = board.enPassantColRow();
        REQUIRE(enPassant.has_value());
        REQUIRE(enPassant->first == col);
        REQUIRE(enPassant->second == fromRow + Board::pawnDirection(c));

        SECTION("Undo also reverts enPassant state") {
            REQUIRE(board.undoMove());
            REQUIRE_FALSE(board.pieceAt(col, toRow).has_value());
            REQUIRE(board.pieceAt(col, fromRow) == pawn);
            REQUIRE_FALSE(board.enPassantColRow().has_value());
        }

        SECTION("Double push from opponent makes that the en passant state") {

            uint8_t col2 = GENERATE(TEST_SOME(range(0, 7)));
            CAPTURE(col2);

            Color opposing = opposite(c);
            REQUIRE(board.colorToMove() == opposing);

            const Piece oppPawn = Piece{Piece::Type::Pawn, opposing};
            uint8_t fromRow2 = Board::pawnHomeRow(opposing);
            uint8_t toRow2 = fromRow2 + Board::pawnDirection(opposing) * 2;
            board.setPiece(col2, fromRow2, oppPawn);
            Move move2 = Move{col2, fromRow2, col2, toRow2, Move::Flag::DoublePushPawn};

            MAKE_VALID_MOVE(move2);

            CAPTURE(board.toFEN());

            REQUIRE_FALSE(board.pieceAt(col2, fromRow2).has_value());
            REQUIRE(board.pieceAt(col2, toRow2) == oppPawn);

            //old pawn still exists
            REQUIRE(board.pieceAt(col, toRow) == pawn);

            auto enPassant2 = board.enPassantColRow();
            REQUIRE(enPassant2.has_value());
            REQUIRE(enPassant2->first == col2);
            REQUIRE(enPassant2->second == fromRow2 + Board::pawnDirection(opposing));


            SECTION("And when undone resets to the previous one") {
                REQUIRE(board.undoMove());
                REQUIRE_FALSE(board.pieceAt(col2, toRow2).has_value());
                REQUIRE(board.pieceAt(col2, fromRow2) == oppPawn);
                REQUIRE(enPassant == board.enPassantColRow());

                //old pawn still exists
                REQUIRE(board.pieceAt(col, toRow) == pawn);

                SECTION("Can also undo the move before that") {
                    REQUIRE(board.undoMove());
                    REQUIRE_FALSE(board.pieceAt(col, toRow).has_value());
                    REQUIRE(board.pieceAt(col, fromRow) == pawn);
                    REQUIRE_FALSE(board.enPassantColRow().has_value());
                }
            }
        }
    }

}


// TODO test PGN generation
// example of pinned piece forcing PGN non ambiguous
// 2k5/3p4/4Q3/8/4q1p1/8/4Q3/4K3 w - - 0 1
