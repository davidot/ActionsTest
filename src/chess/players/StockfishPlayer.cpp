#include "StockfishPlayer.h"

namespace Chess {

    std::unique_ptr<PlayerGameState> StockfishPlayer::startGame(Chess::Color) const {
        return std::unique_ptr<PlayerGameState>();
    }

    std::string StockfishPlayer::name() const {
        return std::string();
    }

    bool StockfishPlayer::isDeterministic() const {
        return false;
    }


}

