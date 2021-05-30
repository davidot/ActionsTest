#include "TrivialPlayers.h"
#include "../../util/Assertions.h"

namespace Chess {

    Move moveAtIndex(const MoveList& list, size_t index) {
        ASSERT(list.size() > 0);
        ASSERT(index < list.size());
        Move mv;

        list.forEachMove([&, i = 0u](const Move& move) mutable {
            if (index == i) {
                mv = move;
            }
            ++i;
        });

        ASSERT(mv.fromPosition != mv.toPosition);
        return mv;
    }

    Move IndexPlayer::pickMove(const Board& board, const MoveList& list) {
        size_t moveIndex = index(board, list);
        return moveAtIndex(list, moveIndex);
    }
    bool IndexPlayer::isDeterministic() const {
        return true;
    }

    std::string RandomPlayer::name() const {
        return "Random";
    }

    size_t RandomPlayer::index(const Board&, const MoveList& list) {
        return randomInt(list.size() - 1);
    }

    size_t wrapAroundIndex(int32_t index, size_t listSize) {
        if (index >= 0) {
            return std::min(size_t(index), listSize - 1u);
        }
        if (-index >= int32_t(listSize)) {
            return 0;
        }
        return listSize + index;
    }

    size_t ConstIndexPlayer::index(const Board&, const MoveList& list) {
        return wrapAroundIndex(val, list.size());
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

    std::unique_ptr<Player> lexicographically(bool ascending, bool from) {
        if (ascending) {
            if (from) {
                return make_stateless<LexicographicallyPlayer<true, true>>();
            }
            return make_stateless<LexicographicallyPlayer<true, false>>();
        }
        if (from) {
            return make_stateless<LexicographicallyPlayer<false, true>>();
        }
        return make_stateless<LexicographicallyPlayer<false, false>>();
    }

    std::unique_ptr<Player> alphabetically(bool ascending) {
        if (ascending) {
            return make_stateless<PGNAlphabeticallyPlayer<true>>();
        }
        return make_stateless<PGNAlphabeticallyPlayer<false>>();
    }

    std::unique_ptr<PlayerGameState> ProgressiveIndexPlayer::startGame(Color color) const {
        return std::make_unique<ProgressiveIndexPlayerState>(m_operation, color, m_startVal);
    }

    std::string ProgressiveIndexPlayer::name() const {
        return baseName + "StartAt" + std::to_string(m_startVal);
    }

    ProgressiveIndexPlayer::ProgressiveIndexPlayer(std::string baseName, std::function<int32_t(int32_t)> mOperation, int32_t mStartVal)
        : baseName(std::move(baseName)),
          m_startVal(mStartVal),
          m_operation(std::move(mOperation)) {
    }

    ProgressiveIndexPlayer::ProgressiveIndexPlayerState::ProgressiveIndexPlayerState(
            const std::function<int32_t(int32_t)>& operation, Color us, int32_t val)
        : val(val),
          operation(operation),
          me(us) {
    }

    Move ProgressiveIndexPlayer::ProgressiveIndexPlayerState::pickMove(const Board&, const MoveList& list) {
        Move mv = moveAtIndex(list, wrapAroundIndex(val, list.size()));
        return mv;
    }

    void ProgressiveIndexPlayer::ProgressiveIndexPlayerState::movePlayed(Move, const Board& board) {
        if (board.colorToMove() != me) {
            val = operation(val);
        }
    }

    std::unique_ptr<Player> indexOp() {
        return std::make_unique<ProgressiveIndexPlayer>(
                "Negated", [](int32_t i) {
                    return -i;
                },
                1);
    }
}// namespace Chess
