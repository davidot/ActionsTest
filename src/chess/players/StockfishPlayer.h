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
            Stockfish m_fish;
        };

        StockfishPlayer(Stockfish::SearchLimit limit);
    private:
        void startProcess();

        Stockfish::SearchLimit m_limit;

    };
}