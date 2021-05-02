#pragma once

#include "Player.h"
#include <random>
#include "../../util/RandomUtil.h"
#include <cstdint>

namespace Chess {

    class RankingPlayer : StatelessPlayer {
    public:
        Move pickMove(const Board& board, const MoveList& list) override;

        virtual int32_t rankMove(Move mv, const Board& board) = 0;

    private:
        struct RankedMove {
            Move mv;
            int32_t ranking;
            uint32_t random;

            bool operator<(const RankedMove& rhs) const;
        };
    };

    class IndexPlayer : public StatelessPlayer {
    public:
        Move pickMove(const Board& board, const MoveList& list) final;

        virtual size_t index(const Board& board, const MoveList& list) = 0;
    };

    class RandomPlayer : public IndexPlayer {
    public:
        size_t index(const Board& board, const MoveList& list) override;

        std::string name() const override;
    };

    class ConstIndexPlayer : public IndexPlayer {
    public:
        explicit ConstIndexPlayer(int32_t i) : val(i) {};

        size_t index(const Board& board, const MoveList& list) override;

        std::string name() const override;
    private:
        int32_t val;
    };

    std::unique_ptr<Player> randomPlayer() {
        return make_stateless<RandomPlayer>();
    }

    std::unique_ptr<Player> indexPlayer(int32_t val) {
        return make_stateless<ConstIndexPlayer>(val);
    }

}