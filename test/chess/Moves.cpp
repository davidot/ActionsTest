#include "TestUtil.h"
#include <catch2/catch.hpp>
#include <chess/Board.h>

#ifdef EXTENDED_TESTS
#include <chess/MoveGen.h>
#define VALIDATE_MOVE(board, move)                                      \
    {                                                                   \
        auto goingToMake = move;                                        \
        auto allMoves = generateAllMoves(board);                        \
        bool found = false;                                             \
        allMoves.forEachMove([&goingToMake, &found](const Move &mm) { \
            if (mm == goingToMake) {                                  \
                REQUIRE_FALSE(found);                                   \
                found = true;                                           \
            }                                                           \
        });                                                             \
        CAPTURE(allMoves.size());                                       \
        REQUIRE(found);                                                 \
    }
#else
#define VALIDATE_MOVE(x, y)
#endif

#define MAKE_VALID_MOVE(...)                                                        \
    {                                                                               \
        VALIDATE_MOVE(board, (__VA_ARGS__));                                          \
        auto currentCol = board.colorToMove();                                      \
        auto numMoves = board.fullMoves();                                          \
        auto halfMovesSI = board.halfMovesSinceIrreversible();                      \
        auto madeMove = board.makeMove(__VA_ARGS__);                                \
        REQUIRE(madeMove);                                                          \
        REQUIRE(board.colorToMove() == opposite(currentCol));                       \
        auto halfMovesSIAfter = board.halfMovesSinceIrreversible();                 \
        REQUIRE((halfMovesSIAfter == 0 || (halfMovesSIAfter == halfMovesSI + 1u))); \
        if (currentCol == Color::Black) {                                           \
            REQUIRE(board.fullMoves() == numMoves + 1u);                            \
        } else {                                                                    \
            REQUIRE(board.fullMoves() == numMoves);                                 \
        }                                                                           \
    }

#define NO_PIECE(col, row)                                \
    do {                                                  \
        INFO("col: " << (col) << " row: " << (row));          \
        REQUIRE(board.pieceAt(col, row) == std::nullopt); \
    } while (false)


using namespace Chess;

TEST_CASE("Apply moves to board", "[chess][move]") {
    Board board = Board::emptyBoard();
    Color c = GENERATE(Color::White, Color::Black);
    CAPTURE(c);

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

        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        const Piece piece = Piece{Piece::Type::Queen, c};
        board.setPiece(fromCol, fromRow, piece);

        MAKE_VALID_MOVE(Move{fromCol, fromRow, toCol, toRow});

        REQUIRE(board.pieceAt(toCol, toRow) == piece);
        REQUIRE(board.pieceAt(fromCol, fromRow) == std::nullopt);
        REQUIRE_FALSE(board.enPassantColRow().has_value());

        SECTION("Undo move") {
            REQUIRE(board.undoMove());
            REQUIRE(board.pieceAt(toCol, toRow) == std::nullopt);
            REQUIRE(board.pieceAt(fromCol, fromRow) == piece);
            REQUIRE_FALSE(board.enPassantColRow().has_value());
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

        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        Piece piece = Piece{Piece::Type::Queen, c};
        board.setPiece(currCol, currRow, piece);

        // we just verify we can get back to the beginning for now
        for (int i = 0; i < steps; i++) {
            auto numMovesPre = board.fullMoves();

            auto [stepCol, stepRow] = moves[i];
            Move mv{currCol, currRow, stepCol, stepRow};
            MAKE_VALID_MOVE(mv);

            REQUIRE(board.pieceAt(currCol, currRow) == std::nullopt);
            REQUIRE(board.pieceAt(stepCol, stepRow) == piece);

            currCol = stepCol;
            currRow = stepRow;
            // get back to our color

            board.makeNullMove();
            // both sides made a move so we must be one further now
            REQUIRE(board.fullMoves() == numMovesPre + 1);
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
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        uint8_t col = GENERATE(TEST_SOME(range(0, 8)));
        uint8_t fromRow = GENERATE(TEST_SOME(range(2, 6)));

        uint8_t toRow = fromRow + Board::pawnDirection(c);

        const Piece piece = Piece{Piece::Type::Pawn, c};
        board.setPiece(col, fromRow, piece);

        MAKE_VALID_MOVE(Move{col, fromRow, col, toRow});

        REQUIRE(board.pieceAt(col, toRow) == piece);

        NO_PIECE(col, fromRow);

        SECTION("Undo move") {
            REQUIRE(board.undoMove());
            NO_PIECE(col, toRow);
            REQUIRE(board.pieceAt(col, fromRow) == piece);
        }
    }

#define CAPTURABLE_TYPES TEST_SOME(values({Piece::Type::Pawn, Piece::Type::Rook, Piece::Type::Knight, Piece::Type::Bishop, Piece::Type::Queen}))

    SECTION("Can capture another piece") {
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        uint8_t fromCol = 1;
        uint8_t fromRow = 1;

        uint8_t toCol = 1;
        uint8_t toRow = 2;

        const Piece piece = Piece{Piece::Type::Queen, c};
        const Piece capturing = Piece{GENERATE(CAPTURABLE_TYPES), opposite(c)};

        board.setPiece(fromCol, fromRow, piece);
        board.setPiece(toCol, toRow, capturing);

        MAKE_VALID_MOVE(Move{fromCol, fromRow, toCol, toRow});

        REQUIRE(board.pieceAt(toCol, toRow) == piece);
        NO_PIECE(fromCol, fromRow);

        SECTION("Undo places exact piece back") {
            REQUIRE(board.undoMove());
            REQUIRE(board.pieceAt(toCol, toRow) == capturing);
            REQUIRE(board.pieceAt(fromCol, fromRow) == piece);
        }
    }

    SECTION("Can perform promotion") {
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        uint8_t col = GENERATE(TEST_SOME(range(0, 7)));
        uint8_t fromRow = Board::pawnHomeRow(opposite(c));
        uint8_t toRow = Board::homeRow(opposite(c));

        const Piece pawn = Piece{Piece::Type::Pawn, c};
        board.setPiece(col, fromRow, pawn);
        Move::Flag flag = GENERATE(Move::Flag::PromotionToQueen, Move::Flag::PromotionToKnight, Move::Flag::PromotionToBishop, Move::Flag::PromotionToRook);
        Move move = Move{col, fromRow, col, toRow, flag};

        MAKE_VALID_MOVE(move);

        NO_PIECE(col, fromRow);

        auto promotedPiece = board.pieceAt(col, toRow);
        REQUIRE(promotedPiece.has_value());
        REQUIRE(promotedPiece->type() == move.promotedType());
        REQUIRE_FALSE(board.enPassantColRow().has_value());

        SECTION("Undo puts back a pawn") {
            REQUIRE(board.undoMove());
            NO_PIECE(col, toRow);
            REQUIRE(board.pieceAt(col, fromRow) == pawn);
            REQUIRE_FALSE(board.enPassantColRow().has_value());
        }
    }

    SECTION("Double push sets en passant") {
        REQUIRE_FALSE(board.enPassantColRow().has_value());

        uint8_t col = GENERATE(TEST_SOME(range(0, 7)));

        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        CAPTURE(col, c);

        uint8_t fromRow = Board::pawnHomeRow(c);
        uint8_t toRow = fromRow + Board::pawnDirection(c) * 2;

        const Piece pawn = Piece{Piece::Type::Pawn, c};
        board.setPiece(col, fromRow, pawn);

        REQUIRE_FALSE(board.enPassantColRow().has_value());

        Move move = Move{col, fromRow, col, toRow, Move::Flag::DoublePushPawn};

        MAKE_VALID_MOVE(move);

        NO_PIECE(col, fromRow);
        REQUIRE(board.pieceAt(col, toRow) == pawn);

        auto enPassant = board.enPassantColRow();
        REQUIRE(enPassant.has_value());
        REQUIRE(enPassant->first == col);
        REQUIRE(enPassant->second == fromRow + Board::pawnDirection(c));

        SECTION("Undo also reverts enPassant state") {
            REQUIRE(board.undoMove());
            NO_PIECE(col, toRow);
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

            NO_PIECE(col2, fromRow2);
            REQUIRE(board.pieceAt(col2, toRow2) == oppPawn);

            //old pawn still exists
            REQUIRE(board.pieceAt(col, toRow) == pawn);

            auto enPassant2 = board.enPassantColRow();
            REQUIRE(enPassant2.has_value());
            REQUIRE(enPassant2->first == col2);
            REQUIRE(enPassant2->second == fromRow2 + Board::pawnDirection(opposing));


            SECTION("And when undone resets to the previous one") {
                REQUIRE(board.undoMove());
                NO_PIECE(col2, toRow2);
                REQUIRE(board.pieceAt(col2, fromRow2) == oppPawn);
                REQUIRE(enPassant == board.enPassantColRow());

                //old pawn still exists
                REQUIRE(board.pieceAt(col, toRow) == pawn);

                SECTION("Can also undo the move before that") {
                    REQUIRE(board.undoMove());
                    NO_PIECE(col, toRow);
                    REQUIRE(board.pieceAt(col, fromRow) == pawn);
                    REQUIRE_FALSE(board.enPassantColRow().has_value());
                }
            }
        }
    }

    SECTION("Can capture with en passant") {
        uint8_t col = GENERATE(TEST_SOME(range(0u, 8u)));
        board = TestUtil::createEnPassantBoard(c, col);

        BoardIndex myCol = col +
                           GENERATE_COPY(filter([col](int8_t i) {
                               return col + i >= 0 && col + i < 8;
                           }, values({1, -1})));
        CAPTURE(col, myCol);

        BoardIndex behindPawn = Board::pawnHomeRow(opposite(c)) + Board::pawnDirection(opposite(c));
        uint8_t enPassantRow =  behindPawn + Board::pawnDirection(opposite(c));

        Piece pawn = Piece{Piece::Type::Pawn, c};
        Piece oppPawn = Piece{Piece::Type::Pawn, opposite(c)};

        REQUIRE(board.pieceAt(col, enPassantRow) == oppPawn);

        board.setPiece(myCol, enPassantRow, pawn);

        Move move = {myCol, enPassantRow, col, behindPawn, Move::Flag::EnPassant};
        MAKE_VALID_MOVE(move);

        REQUIRE_FALSE(board.enPassantColRow().has_value());
        NO_PIECE(myCol, enPassantRow);

        REQUIRE(board.pieceAt(col, behindPawn) == pawn);
        NO_PIECE(col, enPassantRow);

        SECTION("Places back pawn after undo") {
            REQUIRE(board.undoMove());

            REQUIRE(board.pieceAt(col, enPassantRow) == oppPawn);
            REQUIRE(board.pieceAt(myCol, enPassantRow) == pawn);

            NO_PIECE(col, behindPawn);
            NO_PIECE(myCol, behindPawn);

            REQUIRE(board.enPassantColRow().has_value());
            REQUIRE(board.enPassantColRow()->first == col);
            REQUIRE(board.enPassantColRow()->second == behindPawn);
        }
    }

    const auto colRights = c == Color::White ? CastlingRight::WhiteCastling : CastlingRight::BlackCastling;

    SECTION("King or rook move invalidates castling rights") {
        bool withOpponent = GENERATE(true, false);
        board = TestUtil::generateCastlingBoard(c, true, true, false, withOpponent);
        const auto rights = board.castlingRights();
        const auto oppColRights = c == Color::Black ? CastlingRight::WhiteCastling : CastlingRight::BlackCastling;

        BoardIndex home = Board::homeRow(c);

        SECTION("If rook moves invalidates only that right") {
            BoardIndex colFrom = GENERATE(Board::queenSideRookCol, Board::kingSideRookCol);
            BoardIndex rowTo = GENERATE(TEST_SOME(range(1, 7)));
            // also dont take opposing rook...

            Move move = {colFrom, home, colFrom, rowTo};

            MAKE_VALID_MOVE(move);

            auto newRights = board.castlingRights();
            CHECK(newRights != rights);
            CHECK((newRights & oppColRights) == (rights & oppColRights));
            // limit to just us
            newRights = newRights & colRights;

            if (colFrom == Board::queenSideRookCol) {
                REQUIRE((newRights & CastlingRight::QueenSideCastling) == CastlingRight::NoCastling);
            } else {
                REQUIRE(colFrom == Board::kingSideRookCol);
                REQUIRE((newRights & CastlingRight::KingSideCastling) == CastlingRight::NoCastling);
            }

            SECTION("Undo sets the castling rights back") {
                REQUIRE(board.undoMove());
                REQUIRE(board.castlingRights() == rights);
            }
        }

        SECTION("If king moves") {
            BoardIndex rowInFront = Board::pawnHomeRow(c);

            BoardIndex rowTo = GENERATE_COPY(home, rowInFront);
            BoardIndex colTo = GENERATE_COPY(filter([=](BoardIndex i){
                                                return rowTo != home || i != Board::kingCol;
                                            },
                                            range(Board::kingCol - 1, Board::kingCol + 1)));

            Move move = {Board::kingCol, home, colTo, rowTo};

            MAKE_VALID_MOVE(move);

            auto newRights = board.castlingRights();
            CHECK(newRights != rights);
            CHECK((newRights & oppColRights) == (rights & oppColRights));

            newRights = newRights & colRights;
            REQUIRE(newRights == CastlingRight::NoCastling);

            SECTION("Undo sets the castling rights back") {
                REQUIRE(board.undoMove());
                REQUIRE(board.castlingRights() == rights);
            }
        }

        if (!withOpponent) {
            SECTION("Color does not affect castling right of opponent") {
                Color other = opposite(c);

                BoardIndex colFrom = GENERATE(Board::queenSideRookCol, Board::kingSideRookCol);
                board.setPiece(colFrom, 4, Piece{Piece::Type::Rook, other});
                board.makeNullMove();

                MAKE_VALID_MOVE(Move{colFrom, 4, colFrom, 5});

                REQUIRE(board.castlingRights() == rights);

                SECTION("Undo keeps the castling rights") {
                    REQUIRE(board.undoMove());
                    REQUIRE(board.castlingRights() == rights);
                }
            }
        } else {
            SECTION("Taking rook with rook invalidates both rights") {
                Color other = opposite(c);

                BoardIndex colFrom = GENERATE(Board::queenSideRookCol, Board::kingSideRookCol);
                board.makeNullMove();

                MAKE_VALID_MOVE(Move{colFrom, Board::homeRow(other), colFrom, home});

                auto newRights = board.castlingRights();
                REQUIRE(newRights != rights);

                if (colFrom == Board::queenSideRookCol) {
                    REQUIRE(newRights == CastlingRight::KingSideCastling);
                } else {
                    REQUIRE(colFrom == Board::kingSideRookCol);
                    REQUIRE(newRights == CastlingRight::QueenSideCastling);
                }

                SECTION("Undo reverts the castling rights") {
                    REQUIRE(board.undoMove());
                    REQUIRE(board.castlingRights() == rights);
                }
            }
        }

        SECTION("Taking a rook also invalidates the castling rights") {
            Color other = opposite(c);

            BoardIndex colFrom = GENERATE(Board::queenSideRookCol, Board::kingSideRookCol);

            BoardIndex rowFrom = Board::homeRow(other) + Board::pawnDirection(other) * 2;

            board.setPiece(colFrom, rowFrom, Piece{Piece::Type::Rook, other});
            board.makeNullMove();

            MAKE_VALID_MOVE(Move{colFrom, rowFrom, colFrom, home});

            auto newRights = board.castlingRights();
            CHECK(newRights != rights);
            CHECK((newRights & oppColRights) == (rights & oppColRights));
            // limit to just us
            newRights = newRights & colRights;

            if (colFrom == Board::queenSideRookCol) {
                REQUIRE((newRights & CastlingRight::QueenSideCastling) == CastlingRight::NoCastling);
            } else {
                REQUIRE(colFrom == Board::kingSideRookCol);
                REQUIRE((newRights & CastlingRight::KingSideCastling) == CastlingRight::NoCastling);
            }

            SECTION("Undo keeps the castling rights") {
                REQUIRE(board.undoMove());
                REQUIRE(board.castlingRights() == rights);
            }
        }

    }

    SECTION("Can castle") {
        bool kingSide = GENERATE(true, false);
        bool queenSide = GENERATE_COPY(filter([=](bool b) { return b || kingSide; }, values({1, 0})));
        bool castlingKingSide = GENERATE_COPY(filter([=](bool b) { return (b && kingSide) || (!b && queenSide); }, values({1, 0})));

        bool withOpponent = GENERATE(true, false);

        CAPTURE(kingSide, queenSide, castlingKingSide, withOpponent);

        board = TestUtil::generateCastlingBoard(c, kingSide, queenSide, false, withOpponent);
        const auto rights = board.castlingRights();

        BoardIndex home = Board::homeRow(c);

        BoardIndex rookCol = castlingKingSide ? Board::kingSideRookCol : Board::queenSideRookCol;

        Move m = {Board::kingCol, home, rookCol, home, Move::Flag::Castling};
        MAKE_VALID_MOVE(m);

        Piece king{Piece::Type::King, c};
        Piece rook{Piece::Type::Rook, c};
        if (castlingKingSide) {
            CHECK(board.pieceAt(Board::kingCol + 2, home) == king);
            CHECK(board.pieceAt(Board::kingCol + 1, home) == rook);
            CHECK((board.pieceAt(Board::queenSideRookCol, home) == rook) == queenSide);
            for (BoardIndex col = 1; col < Board::size; col++) {
                if (col == Board::kingCol + 1) {
                    ++col;
                    continue;
                }
                NO_PIECE(col, home);
            }
        } else {
            CHECK(board.pieceAt(Board::kingCol - 2, home) == king);
            CHECK(board.pieceAt(Board::kingCol - 1, home) == rook);
            CHECK((board.pieceAt(Board::kingSideRookCol, home) == rook) == kingSide);
            for (BoardIndex col = 0; col < Board::size - 1; col++) {
                if (col == Board::kingCol - 2) {
                    ++col;
                    continue;
                }
                NO_PIECE(col, home);
            }
        }

        // also resets castling rights on that side
        REQUIRE((board.castlingRights() & colRights) == CastlingRight::NoCastling);

        SECTION("Places back pieces after undo") {
            REQUIRE(board.undoMove());
            REQUIRE(board.pieceAt(Board::kingCol, home) == king);
            REQUIRE((board.pieceAt(Board::kingSideRookCol, home) == rook) == kingSide);
            REQUIRE((board.pieceAt(Board::queenSideRookCol, home) == rook) == queenSide);
            REQUIRE(board.castlingRights() == rights);
        }

    }

    SECTION("Half moves since irreversible is updated correctly") {

        uint32_t makeHalfMoves = GENERATE(TEST_SOME(values({0, 1, 2, 5, 13, 99, 98})));

        auto setHalfMoves = [&] {
          if (board.colorToMove() != c) {
              board.makeNullMove();
          }
          std::string fen = board.toFEN();
          auto pos = fen.rfind('-');
          REQUIRE(pos != std::string::npos);
          REQUIRE(fen[pos + 1] == ' ');
          REQUIRE(fen[pos + 3] == ' ');
          fen.replace(pos + 2, 1, std::to_string(makeHalfMoves));

          auto eb = Board::fromFEN(fen);
          if (!eb) {
              WARN(eb.error());
              REQUIRE(eb);
          }
          board = eb.extract();
          REQUIRE(board.halfMovesSinceIrreversible() == makeHalfMoves);
        };

        SECTION("Pawn move resets halfmove counter to 0") {
            board = Board::standardBoard();
            REQUIRE(board.halfMovesSinceIrreversible() < 10);

            setHalfMoves();

            Move mv(0, Board::pawnHomeRow(c), 0, Board::pawnHomeRow(c) + Board::pawnDirection(c));
            MAKE_VALID_MOVE(mv);

            REQUIRE(board.halfMovesSinceIrreversible() == 0);

            SECTION("Undo sets it back") {
                REQUIRE(board.undoMove());
                REQUIRE(board.halfMovesSinceIrreversible() == makeHalfMoves);
            }

        }

        SECTION("Capture resets half move counter") {
            board.setPiece(0, 0, Piece{Piece::Type::Queen, c});
            board.setPiece(1, 1, Piece{Piece::Type::Queen, opposite(c)});

            setHalfMoves();

            Move mv{0, 0, 1, 1};
            MAKE_VALID_MOVE(mv);

            REQUIRE(board.halfMovesSinceIrreversible() == 0);

            SECTION("Undo sets it back") {
                REQUIRE(board.undoMove());
                REQUIRE(board.halfMovesSinceIrreversible() == makeHalfMoves);
                REQUIRE(board.pieceAt(1, 1) == Piece{Piece::Type::Queen, opposite(c)});
            }
        }
    }
}

TEST_CASE("Generates correct SAN notation for moves") {

}

// TODO test PGN generation
// example of pinned piece forcing PGN non ambiguous
// 2k5/3p4/4Q3/8/4q1p1/8/4Q3/4K3 w - - 0 1
