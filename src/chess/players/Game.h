#pragma once

#include <memory>
#include "Player.h"

namespace Chess {

    enum class GameResult {
        WhiteWin,
        BlackWin,
        Draw
    };

    GameResult playGame(const Player* whitePlayer, const Player* blackPlayer);

}