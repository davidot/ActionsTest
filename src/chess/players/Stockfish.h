#pragma once

#include "../Move.h"
#include "../Types.h"
#include <string>
#include <memory>

namespace util {
    class SubProcess;
}

namespace Chess {
    class Stockfish {
    public:
        struct SearchLimit {
            enum LimitType {
                Nodes,
                MoveTime,
                Depth
            } type;
            uint32_t val = 0;

            std::string toLimit() const;

            static SearchLimit nodes(uint32_t nodes);

            static SearchLimit moveTime(uint32_t time);

            static SearchLimit depth(uint32_t depth);

        private:
            SearchLimit(LimitType tp, uint32_t val);
        };

        struct MoveResult {
            std::string bestMove;
            int32_t score;
        };

        MoveResult bestMove(const Board& board) const;

        explicit Stockfish(SearchLimit limit, int difficulty = 20);

        ~Stockfish();
    private:
        std::string m_limitedGo;
        std::unique_ptr<util::SubProcess> m_proc;

    };
}