#include "chess/Move.h"
#include "chess/Piece.h"
#include <chess/Board.h>
#include <chess/MoveGen.h>
#include <chess/players/Game.h>
#include <chess/players/TrivialPlayers.h>
#include <cstddef>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <util/RandomUtil.h>

int main(int argc, char** argv) {
    int status = 0;

//    auto rand = Chess::randomPlayer();

//    auto players = {
//            Chess::indexPlayer(0),
//            Chess::indexPlayer(1),
//            Chess::indexPlayer(8),
//            Chess::indexPlayer(-1),
//            Chess::indexPlayer(-4),
//    };
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

    auto minOpp = Chess::minOpponentMoves();
    auto maxOpp = Chess::maxOpponentMoves();
    auto lexi1 = Chess::lexicographically(true, true);
    auto lexi2 = Chess::lexicographically(false, true);
    auto lexi3 = Chess::lexicographically(true, false);
    auto lexi4 = Chess::lexicographically(false, false);
    auto alpha = Chess::alphabetically(true);
    auto alphaR = Chess::alphabetically(false);
    auto rand = Chess::randomPlayer();


    auto playGame = [](auto& white, auto& black) {
        auto result = Chess::playGame(white, black);
        std::cout << white->name() << " vs " << black->name() << "\n"
                  << result.stringifyResult() << '\n'
                  << "PGN: " << result.pgn << "\n\n";
        return result;
    };


//    playGame(minOpp, minOpp);
//    playGame(maxOpp, minOpp);
//    playGame(minOpp, maxOpp);
    playGame(maxOpp, maxOpp);

    std::vector<std::reference_wrapper<std::unique_ptr<Chess::Player>>> players = { lexi1, lexi2, lexi3, lexi4, alpha, alphaR};

//    for (auto& w : players) {
//        for (auto& b : players) {
//            playGame(w.get(), b.get());
//        }
//    }

    return status;
}
