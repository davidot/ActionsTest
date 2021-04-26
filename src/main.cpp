#include <chess/Board.h>
#include <chess/MoveGen.h>
#include <iostream>
#include <random>
#include <sstream>
#include <util/Assertions.h>
#include <util/RandomUtil.h>

int main(int argc, char** argv) {
    int status = 0;

    Chess::Board board = Chess::Board::standardBoard();

    Chess::MoveList moves = Chess::generateAllMoves(board);

    std::string seed;
    auto orng = util::seedRNGFromString<std::mt19937_64>(seed, 64);
    if (!orng.has_value()) {
        std::cout << "Wrong seed\n";
        return 2;
    }
    auto rng = orng.value();

    std::cout << "Using seed:\n" << seed << '\n';

    size_t moveNum = 0;
    std::stringstream pgn;

    std::optional<Chess::Color> capturer;
    bool both = false;

    if (argc > 1) {
        char first = argv[1][0];
        capturer = (first == 'w' || first == 'W') ? Chess::Color::White : Chess::Color::Black;
        if (argv[1] == std::string("wb")) {
            both = true;
            std::cout << "Using capturer: both\n";
        } else {
            std::cout << "Using capturer: " << capturer.value() << '\n';
        }
    }

    while (moves.size() > 0 && moveNum < 1000) {
        size_t moveIndex = std::uniform_int_distribution<size_t>(0, moves.size() - 1)(rng);

        Chess::Move mv;

        if (board.colorToMove() == capturer || both) {
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

        if (moves.size() > 0 && board.isDrawn()) {
            if (board.halfMovesSinceIrreversible() > 99 && board.positionRepeated() > 2) {
                std::cout << "Draw by no irreversible halfmove !And! repetition\n";
            } else if (board.halfMovesSinceIrreversible() > 99) {
                std::cout << "Draw by no irreversible halfmove for 100 moves\n";
            } else if (board.positionRepeated() > 2) {
                std::cout << "Draw by a 3 times repeated position\n";
            } else {
                std::cout << "Draw by ????\n";
                status = 4;
            }
            break;
        }

        if (board.countPieces(Chess::Color::White) == 1 && board.countPieces(Chess::Color::Black) == 1) {
            std::cout << "Draw by just kings left (boring)\n";

            bool white = false;
            bool black = false;

            for (int col = 0; col < 8; col++) {
                for (int row = 0; row < 8; row++) {
                    auto p = board.pieceAt(col, row);
                    if (p.has_value()) {
                        if (p->type() != Chess::Piece::Type::King) {
                            std::cout << "Non king at" << col << ',' << row << " ??\n";
                            status = 5;
                        } else {
                            if (p->color() == Chess::Color::Black) {
                                if (black) {
                                    std::cout << "Two black kings??? at" << col << ',' << row << " ??\n";
                                    status = 6;
                                }
                                black = true;
                            } else {
                                if (white) {
                                    std::cout << "Two white kings??? at" << col << ',' << row << " ??\n";
                                    status = 6;
                                }
                                white = true;
                            }
                        }
                    }
                }
            }

            if (!white || !black) {
                std::cout << "No king(s) left black: " << black << " white: " << white << " ??\n";
                status = 7;
            }

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

    return status;
}
