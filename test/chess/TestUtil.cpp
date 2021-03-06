#include "TestUtil.h"
#include <chess/Board.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generator_exception.hpp>
#include <catch2/internal/catch_random_number_generator.hpp>
#include <catch2/internal/catch_enforce.hpp>
#include <catch2/internal/catch_context.hpp>
#include <random>

namespace Catch::Generators::SampleDetails {

    size_t random_uniform_int(size_t min, size_t max) {
        return std::uniform_int_distribution<size_t>(min, max)(Catch::rng());
    }

    void fail(const char *msg) {
        Catch::throw_exception(GeneratorException(msg));
    }
}

namespace TestUtil {
    Chess::Board makeCastlingBoard(Chess::Color toMove, bool kingSide, bool queenSide, bool withOppositeRook, bool opponent) {
        using namespace Chess;
        const uint8_t homeRow = Board::homeRow(toMove);
        const uint8_t queenSideRook = Board::queenSideRookCol;
        const uint8_t kingSideRook = Board::kingSideRookCol;
        const uint8_t kingCol = Board::kingCol;

        Board board = Board::emptyBoard();
        if (board.colorToMove() != toMove) {
            board.makeNullMove();
        }

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

        if (opponent) {
            Color opp = opposite(toMove);
            BoardIndex myHome = Board::homeRow(opp);
            board.setPiece(kingCol, myHome, Piece{Piece::Type::King, opp});
            board.setPiece(kingSideRook, myHome, Piece{Piece::Type::Rook, opp});
            board.setPiece(queenSideRook, myHome, Piece{Piece::Type::Rook, opp});

            if (opp == Color::White) {
                castles = "KQ" + castles;
            } else {
                castles = castles + "kq";
            }
        }

        std::string baseFEN = board.toFEN();
        auto loc = baseFEN.rfind("- - ");
        REQUIRE(loc != std::string::npos);
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
        return board;
    }

    Chess::Board generateCastlingBoard(Chess::Color c, bool kingSide, bool queenSide, bool withOpposite, bool withOpponent) {
        using namespace Chess;
        struct BoardCache {
            Color col;
            bool king;
            bool queen;
            bool opposite;
            bool opponent;
            Board board;
        };
        static std::vector<BoardCache> boards;
        if (boards.empty()) {
            boards.reserve(10);
        }

        for (const BoardCache& cache : boards) {
            if (cache.col == c
                && cache.king == kingSide
                && cache.queen == queenSide
                && cache.opposite == withOpposite) {
                return cache.board;
            }
        }

        boards.push_back(BoardCache{c, kingSide, queenSide, withOpposite, withOpponent,
                                    makeCastlingBoard(c, kingSide, queenSide, withOpposite, withOpponent)});

        return boards.back().board;
    }



    Chess::Board createEnPassantBoard(Chess::Color c, Chess::BoardIndex col) {
        using namespace Chess;
        const BoardOffset offset = Board::pawnDirection(c);
        const BoardIndex endRow = Board::pawnHomeRow(opposite(c));// reverse of start
        const BoardIndex enPassantRowOther = endRow - offset;
        const BoardIndex rowAfterDoublePushOther = enPassantRowOther - offset;

        Board board = Board::emptyBoard();
        if (board.colorToMove() != c) {
            board.makeNullMove();
        }

        // setup en passant square via FEN (we do not want a method to set this)
        board.setPiece(col, rowAfterDoublePushOther, Piece{Piece::Type::Pawn, opposite(c)});
        std::string baseFEN = board.toFEN();
        auto loc = baseFEN.rfind("- ");
        REQUIRE(loc != baseFEN.size());
        REQUIRE(loc != std::string::npos);
        baseFEN.replace(loc, 1, Board::columnRowToSAN(col, enPassantRowOther));
        auto eBoard = Board::fromFEN(baseFEN);
        REQUIRE(eBoard);
        board = eBoard.extract();
        REQUIRE(board.colorToMove() == c);
        REQUIRE(board.enPassantColRow() == std::make_pair(col, enPassantRowOther));

        return board;
    }

}


namespace std { // NOLINT(cert-dcl58-cpp)
    std::ostream &operator<<(std::ostream &os, const std::optional<Chess::Piece> &piece) {
        if (piece.has_value()) {
            os << piece.value();
        } else {
            os << "No piece";
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const std::optional<Chess::Move> &mv) {
        if (mv.has_value()) {
            os << mv->toSANSquares();
            if (mv->flag != Chess::Move::Flag::None) {
                os << " [" << static_cast<unsigned>(mv->flag) << ']';
            }
        } else {
            os << "No move";
        }
        return os;
    }
}
