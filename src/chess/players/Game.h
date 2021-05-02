#pragma once

#include <memory>
#include "Player.h"

namespace Chess {

    struct GameResult {
        enum class State {
            InProgress,
            WhiteWin,
            BlackWin,
            Draw,
        } state;

        std::string pgn;
    };

    GameResult playGame(const Player* whitePlayer, const Player* blackPlayer);

    GameResult playGame(const std::unique_ptr<Player>& white, const std::unique_ptr<Player>& black);

}
