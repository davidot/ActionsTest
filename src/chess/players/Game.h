#pragma once

#include "Player.h"
#include <memory>

namespace Chess {

    struct GameResult {
        enum class Final {
            InProgress,
            WhiteWin,
            BlackWin,
            Draw,
        };

        constexpr static auto BlackResult = 1;

        enum class SpecificResult {
            InProgress = 0,
            BlackWin,
            WhiteWin = BlackWin + BlackResult,
            WhiteStaleMate,
            BlackStaleMate = WhiteStaleMate + BlackResult,
            WhiteRepetition,
            BlackRepetition = WhiteRepetition + BlackResult,
            WhiteNoIrreversibleMoveMade,
            BlackNoIrreversibleMoveMade = WhiteNoIrreversibleMoveMade + BlackResult,
        } specificRes;

        std::string pgn;

        [[nodiscard]] Final final() const;

        [[nodiscard]] std::string stringifyResult() const;
    };

    GameResult playGame(const Player* whitePlayer, const Player* blackPlayer);

    GameResult playGame(const std::unique_ptr<Player>& white, const std::unique_ptr<Player>& black);

}// namespace Chess
