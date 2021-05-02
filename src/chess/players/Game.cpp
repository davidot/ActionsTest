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
        result.pgn = pgn.str();
        Color toMove = board.colorToMove();

        if (list.size() > 0) {
            if (board.isDrawn()) {
                if (board.positionRepeated() > 2) {
                    result.specificRes = GameResult::SpecificResult::WhiteRepetition;
                } else {
                    ASSERT(board.halfMovesSinceIrreversible() > 99);
                    result.specificRes = GameResult::SpecificResult::WhiteNoIrreversibleMoveMade;
                }
            } else {
                result.specificRes = GameResult::SpecificResult::InProgress;
            }
        } else {
            if (list.isStaleMate()) {
                result.specificRes = GameResult::SpecificResult::WhiteStaleMate;
            } else {
                ASSERT(list.isCheckMate());
                // black win since if black to move it becomes white win!
                result.specificRes = GameResult::SpecificResult::BlackWin;
            }
        }

        if (toMove == Color::Black) {
            result.specificRes = static_cast<GameResult::SpecificResult>(
                    static_cast<decltype(GameResult::BlackResult)>(result.specificRes) + GameResult::BlackResult);
        }

        return result;
    }

    GameResult playGame(const std::unique_ptr<Player> &white,
                               const std::unique_ptr<Player> &black) {
        return playGame(white.get(), black.get());
    }

    std::string GameResult::stringifyResult() const {
        switch (specificRes) {
            case SpecificResult::InProgress:
                return "";
            case SpecificResult::WhiteWin:
                return "White win";
            case SpecificResult::BlackWin:
                return "Black win";
            case SpecificResult::WhiteStaleMate:
                return "White stalemate";
            case SpecificResult::BlackStaleMate:
                return "Black stalemate";
            case SpecificResult::WhiteRepetition:
                return "White repetition";
            case SpecificResult::BlackRepetition:
                return "Black repetition";
            case SpecificResult::WhiteNoIrreversibleMoveMade:
                return "White no irreversible move made";
            case SpecificResult::BlackNoIrreversibleMoveMade:
                return "Black no irreversible move made";
        }
    }

    GameResult::Final GameResult::final() const {
        switch (specificRes) {
            case SpecificResult::InProgress:
                return Final::InProgress;
            case SpecificResult::WhiteWin:
                return Final::WhiteWin;
            case SpecificResult::BlackWin:
                return Final::BlackWin;
            case SpecificResult::WhiteStaleMate:
            case SpecificResult::BlackStaleMate:
            case SpecificResult::WhiteRepetition:
            case SpecificResult::BlackRepetition:
            case SpecificResult::WhiteNoIrreversibleMoveMade:
            case SpecificResult::BlackNoIrreversibleMoveMade:
                return Final::Draw;
        }
    }

}
