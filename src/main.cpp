#include <chess/Board.h>
#include <chess/MoveGen.h>
#include <iostream>
#include <random>
#include <sstream>
#include <util/Assertions.h>
#include <util/RandomUtil.h>

int main(int argc, char** argv) {
    Chess::Board board = Chess::Board::standardBoard();

    std::cout << "Start fen: " << board.toFEN() << '\n';

    Chess::MoveList moves = Chess::generateAllMoves(board);

    std::string seed;
    auto orng = util::seedRNGFromString<std::mt19937_64>(seed, 64);
    ASSERT(orng.has_value());
    auto rng = orng.value();

    std::cout << "Using seed:\n" << seed << '\n';

    size_t moveNum = 0;
    std::stringstream pgn;

    std::optional<Chess::Color> capturer;

    if (argc > 1) {
        char first = argv[1][0];
        capturer = (first == 'w' || first == 'W') ? Chess::Color::White : Chess::Color::Black;
        std::cout << "Using capturer: " << capturer.value() << '\n';
    }

    while (moves.size() > 0 && moveNum < 1000) {
        size_t moveIndex = std::uniform_int_distribution<size_t>(0, moves.size() - 1)(rng);

        Chess::Move mv;

        if (board.colorToMove() == capturer) {
            // prefer captures
            int count = 0;
            moves.forEachMove([&](const Chess::Move& move){
                auto [toCol, toRow] = move.colRowToPosition();
                if (board.pieceAt(toCol, toRow) != std::nullopt) {
                    count++;
                }
            });

            if (count > 0) {
                moveIndex = std::uniform_int_distribution<size_t>(0, count - 1)(rng);
                moves.forEachFilteredMove([&](const Chess::Move& move){
                  auto [toCol, toRow] = move.colRowToPosition();
                  return board.pieceAt(toCol, toRow) != std::nullopt;
                }, [&mv, moveIndex, i = 0u](const Chess::Move& move) mutable {
                  if (i++ == moveIndex) {
                      mv = move;
                  }
                });
            }
        }

        if (mv.fromPosition == mv.toPosition) {
            moves.forEachMove([&mv, moveIndex, i = 0u](const Chess::Move& move) mutable {
              if (i++ == moveIndex) {
                  mv = move;
              }
            });
        }

        if (board.colorToMove() == Chess::Color::White) {
            pgn << board.fullMoves() << ". ";
        }

        pgn << board.moveToSAN(mv) << " ";

        board.makeMove(mv);

        moveNum++;
        moves = Chess::generateAllMoves(board);

        if (moves.size() > 0 && board.halfMovesSinceIrreversible() >= 150) {
            std::cout << "Draw by no irreversible halfmove\n";
            break;
        }

    }

    if (moves.isCheckMate()) {
        std::cout << "Checkmate!\n";
    } else if (moves.isStaleMate()) {
        std::cout << "Stalemate!\n";
    }

    std::cout << "Finished game!\n" << board.toFEN() << " after " << moveNum << " moves\n";

    std::cout << pgn.str();
}
