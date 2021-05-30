#include "chess/Move.h"
#include "chess/Piece.h"
#include <chess/Board.h>
#include <chess/MoveGen.h>
#include <chess/players/Game.h>
#include <chess/players/Stockfish.h>
#include <chess/players/TrivialPlayers.h>
#include <cstddef>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <util/RandomUtil.h>
#include <chess/players/StockfishPlayer.h>

std::optional<std::string> envVar(const std::string& name) {
    char* value = getenv(name.c_str());
    if (value == nullptr) {
        return std::nullopt;
    }
    return value;
}

int main(int argc, char** argv) {
    bool hasStockfish = false;

    {
        auto stockfishLoc = envVar("STOCKFISH_PATH");
        if (stockfishLoc.has_value()) {
            Chess::setStockfishLocation(*stockfishLoc);
            hasStockfish = true;
        }
    }

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (argv[i] == std::string("--stockfish")) {
                i++;
                if (i >= argc) {
                    std::cout << "Stockfish argument missing value\n";
                    return 1;
                }
                Chess::setStockfishLocation(argv[i]);
                hasStockfish = true;
            }
        }
    }

    int status = 0;

//    auto rand = Chess::randomPlayer();

    auto players = {
            Chess::indexPlayer(0),
            Chess::indexPlayer(1),
            Chess::indexPlayer(8),
            Chess::indexPlayer(-1),
            Chess::indexPlayer(-4), // draw against depth 4 sf
            Chess::minOpponentMoves(),
            Chess::maxOpponentMoves(),
            Chess::lexicographically(true, true),
            Chess::lexicographically(false, true),
            Chess::lexicographically(true, false),
            Chess::lexicographically(false, false),
            Chess::alphabetically(true),
            Chess::alphabetically(false),
            Chess::randomPlayer(),
    };
//
//    for (const auto& white : players) {
//        for (const auto& black : players) {
//            auto res = Chess::playGame(white, black);
//            if (res.final() != Chess::GameResult::Final::Draw) {
//                std::cout << "Non draw! "
//                          << (res.final() == Chess::GameResult::Final::BlackWin ? "Black" : "White")
//                          << " won\n"
//                          << white->name() << " vs " << black->name() << "\n"
//                          << "PGN: " << res.pgn << '\n';
//            }
//        }
//    }

    auto playGame = [](auto& white, auto& black) {
        auto result = Chess::playGame(white, black);
        std::cout << white->name() << " vs " << black->name() << "\n"
                  << result.stringifyResult() << '\n'
                  << "PGN: " << result.pgn << "\n\n";
        return result;
    };


    if (hasStockfish) {
        auto limit = Chess::Stockfish::SearchLimit::depth(4);
        auto stockfishGood = Chess::stockfish(limit);
        for (auto& white : players) {
            playGame(white, stockfishGood);
        }

        return 0;
    }

    return status;
}
