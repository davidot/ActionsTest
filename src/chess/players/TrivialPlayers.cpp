#include "TrivialPlayers.h"
#include "../../util/Assertions.h"
#include "../MoveGen.h"

namespace Chess {

    Move IndexPlayer::pickMove(const Board& board, const MoveList& list) {
        ASSERT(list.size() > 0);
        size_t moveIndex = index(board, list);

        ASSERT(moveIndex < list.size());
        Move mv;

        list.forEachMove([&, i = 0u](const Move& move) mutable {
          if (moveIndex == i) {
              mv = move;
          }
          ++i;
        });

        ASSERT(mv.fromPosition != mv.toPosition);
        return mv;
    }

    std::string RandomPlayer::name() const {
        return "Random";
    }

    size_t RandomPlayer::index(const Board&, const MoveList& list) {
        return randomInt(list.size() - 1);
    }

    size_t ConstIndexPlayer::index(const Board&, const MoveList& list) {
        if (val >= 0) {
            return std::min(size_t(val), list.size() - 1u);
        }
        if (-val >= int32_t(list.size())) {
            return 0;
        }
        return list.size() + val;
    }

    std::string ConstIndexPlayer::name() const {
        return "Const index: " + std::to_string(val);
    }

    std::unique_ptr<Player> randomPlayer() {
        return make_stateless<RandomPlayer>();
    }

    std::unique_ptr<Player> indexPlayer(int32_t val) {
        return make_stateless<ConstIndexPlayer>(val);
    }

    std::unique_ptr<Player> minOpponentMoves() {
        return make_stateless<LeastOpponentMoves>();
    }

    std::unique_ptr<Player> maxOpponentMoves() {
        return make_stateless<MostOpponentMoves>();
    }

    uint32_t randomInt(uint32_t upperBound, uint32_t lowerBound) {
        static std::string seed = "";
        static auto rng = util::seedRNGFromString<std::mt19937>(seed, 64).value();

        return std::uniform_int_distribution<uint32_t>(lowerBound, upperBound)(rng);
    }
}

