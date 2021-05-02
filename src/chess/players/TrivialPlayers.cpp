#include "TrivialPlayers.h"
#include "../../util/Assertions.h"
#include "../MoveGen.h"

namespace Chess {

    Move RankingPlayer::pickMove(const Board& board, const MoveList& list) {
        // TODO somehow log every seed?
        static std::uniform_int_distribution<uint32_t> fullRangeDist{};
        static auto rng = util::seedRNG<std::mt19937>();

        ASSERT(list.size() > 0);

        std::vector<RankedMove> ranked(list.size());

        size_t index = 0;

        list.forEachMove([&](Move mv) {
            RankedMove& rMove = ranked[index];
            rMove.mv = mv;
            rMove.random = fullRangeDist(rng);
            board.moveExcursion(mv, [&](const Board& board) {
              rMove.ranking = rankMove(mv, board);
            });

        });

        return std::max_element(ranked.begin(), ranked.end())->mv;
    }

    bool RankingPlayer::RankedMove::operator<(const RankingPlayer::RankedMove& rhs) const {
        if (ranking == rhs.ranking) {
            return random < rhs.random;
        }
        return ranking < rhs.ranking;
    }

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
        static auto rng = util::seedRNG<std::mt19937>();
        return std::uniform_int_distribution<size_t>(0, list.size() - 1)(rng);
    }

    size_t ConstIndexPlayer::index(const Board&, const MoveList& list) {
        if (val >= 0) {
            return std::min(size_t(val), list.size());
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
}

