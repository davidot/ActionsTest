#include <iostream>
#include "StockfishPlayer.h"
#include "../MoveGen.h"
#include "../../util/Assertions.h"

namespace Chess {

    std::unique_ptr<PlayerGameState> StockfishPlayer::startGame(Chess::Color) const {
        return std::make_unique<StockfishGame>(m_limit, m_difficulty);
    }

    std::string StockfishPlayer::name() const {
        return "Stockfish " + m_limit.toLimit();
    }

    bool StockfishPlayer::isDeterministic() const {
        return false;
    }


    Move StockfishPlayer::StockfishGame::pickMove(const Board &board, const MoveList &list) {
        auto result = stockfish.bestMove(board);
        Move mv;
        list.hasMove([&mv, &bestMove = result.bestMove](const Move& move) {
            if (move.toSANSquares() == bestMove) {
                mv = move;
                return true;
            }
            return false;
        });
        ASSERT(mv.fromPosition != mv.toPosition);
        return mv;
    }

    StockfishPlayer::StockfishGame::StockfishGame(Stockfish::SearchLimit limit, int difficulty)
        : stockfish(limit, difficulty) {
    }

    StockfishPlayer::StockfishPlayer(Stockfish::SearchLimit limit, int difficulty)
        : m_limit(limit), m_difficulty(difficulty) {
    }

    std::unique_ptr<Player> stockfish(Stockfish::SearchLimit limit, int difficulty) {
        return std::make_unique<StockfishPlayer>(limit, difficulty);
    }
}

