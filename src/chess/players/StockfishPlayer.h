#pragma once

#include "Player.h"
#include "Stockfish.h"

namespace Chess {

    class StockfishPlayer : public Player {
    public:
        std::unique_ptr<PlayerGameState> startGame(Color color) const override;
        std::string name() const override;
        bool isDeterministic() const override;

        struct StockfishGame : public PlayerGameState {
            Move pickMove(const Board& board, const MoveList& list) override;

            explicit StockfishGame(Stockfish::SearchLimit limit, int difficulty);

        private:
            Stockfish stockfish;
        };

        explicit StockfishPlayer(Stockfish::SearchLimit limit, int difficulty = 20);

    private:
        Stockfish::SearchLimit m_limit;
        int m_difficulty;
    };

    std::unique_ptr<Player> stockfish(Stockfish::SearchLimit limit, int difficulty = 20);
}// namespace Chess
