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


    auto playGame = [](auto& white, auto& black) {
        auto result = Chess::playGame(white, black);
        std::cout << white->name() << " vs " << black->name() << "\n"
                  << result.stringifyResult() << '\n'
                  << "PGN: " << result.pgn << "\n\n";
    };


//    playGame(minOpp, minOpp);
//    playGame(maxOpp, minOpp);
    playGame(minOpp, maxOpp);
//    playGame(maxOpp, maxOpp);

    return status;
}
