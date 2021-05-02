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
            board.makeMove(mv);
            whiteState->movePlayed(mv, board);
            blackState->movePlayed(mv, board);

            list = generateAllMoves(board);
        }

        if (board.isDrawn() || list.isStaleMate()) {
            return GameResult::Draw;
        }
        ASSERT(list.isCheckMate());
        if (board.colorToMove() == Color::White) {
            return GameResult::BlackWin;
        }
        return GameResult::WhiteWin;
    }
}
