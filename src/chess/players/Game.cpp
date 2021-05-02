#include <sstream>
#include "Game.h"
#include "../../util/Assertions.h"
#include "../MoveGen.h"

namespace Chess {

    GameResult playGame(const Player* whitePlayer, const Player* blackPlayer) {
        ASSERT(whitePlayer);
        ASSERT(blackPlayer);
        Board board = Board::standardBoard();
        auto whiteState = whitePlayer->startGame(Color::White);
        auto blackState = blackPlayer->startGame(Color::White);

        std::ostringstream pgn;

        MoveList list = generateAllMoves(board);

        while (list.size() && !board.isDrawn()) {
            Color toMove = board.colorToMove();
            Move mv;
            if (toMove == Color::White) {
                mv = whiteState->pickMove(board, list);
            } else {
                mv = blackState->pickMove(board, list);
            }

            ASSERT(list.contains(mv));
            ASSERT(mv.fromPosition != mv.toPosition);

            if (board.colorToMove() == Chess::Color::White) {
                pgn << board.fullMoves() << ". ";
            }
            pgn << board.moveToSAN(mv, list) << " ";

            board.makeMove(mv);
            whiteState->movePlayed(mv, board);
            blackState->movePlayed(mv, board);

            list = generateAllMoves(board);
        }


        GameResult result;


        if (board.isDrawn() || list.isStaleMate()) {
            result.state = GameResult::State::Draw;
        } else if (list.isCheckMate()) {
            if (board.colorToMove() == Color::White) {
                result.state = GameResult::State::BlackWin;
            } else {
                result.state = GameResult::State::WhiteWin;
            }
        }
        result.pgn = pgn.str();
        return result;
    }

    GameResult playGame(const std::unique_ptr<Player> &white,
                               const std::unique_ptr<Player> &black) {
        return playGame(white.get(), black.get());
    }

}
