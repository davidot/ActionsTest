#pragma once

#include "../../util/RandomUtil.h"
#include "../MoveGen.h"
#include "Player.h"
#include <iostream>
#include <cstdint>
#include <random>
#include <functional>

namespace Chess {
#ifdef __MINGW32__
#warning Beyond dangerous macro!!
#define Ranking typename
#else
    template<typename T>
    concept Ranking = requires(T r) {
        r < r;
    };
#endif

    // Note you cannot have ties!
    template<Ranking R, typename Compare = std::less<>>
    class RankingPlayer : public StatelessPlayer {
    public:
        virtual R rankMove(Move mv, const Board& board) = 0;

        bool isDeterministic() const override {
            return true;
        }

    private:
        struct RankedMove {
            Move mv;
            R ranking;
            // TODO: somehow be able to have multiple / other ranking things?
//            int32_t ranking;
//            uint32_t random;

            // Note you cannot have ties!
            bool operator<(const RankingPlayer::RankedMove& rhs) const {
                return Compare{}(ranking, rhs.ranking);
            }

        };

    public:
        Move pickMove(const Board& board, const MoveList& list) final {
            std::vector<RankedMove> ranked(list.size());

            size_t index = 0;

            list.forEachMove([&](Move mv) {
              ranked[index] = {
                  mv,
                  rankMove(mv, board)
              };
              ++index;
            });

            auto best = std::min_element(ranked.begin(), ranked.end());
            return best->mv;
        }
    };

    template<Ranking R, typename Compare = std::less<>>
    struct RankingWithRandom {
        R ranking;
        uint32_t random;

        friend bool operator<(const RankingWithRandom<R, Compare>& lhs, const RankingWithRandom<R, Compare>& rhs) {
            if (Compare{}(lhs.ranking, rhs.ranking)) {
                return true;
            } else if (Compare{}(rhs.ranking, lhs.ranking)) {
                return false;
            }
            return lhs.random < rhs.random;
        }
    };

    uint32_t randomInt(uint32_t upperBound = std::numeric_limits<uint32_t>::max(), uint32_t lowerBound = 0);

    template<Ranking R, typename Compare = std::less<>>
    class EvaluateAfterMovePlayer : public RankingPlayer<RankingWithRandom<R, Compare>> {
    public:
        RankingWithRandom<R, Compare> rankMove(Move mv, const Board& board) override {
            return {
                board.moveExcursion(mv, [&](const Board& postMoveBoard) {
                    return ranking(mv, postMoveBoard);
                }),
                randomInt()
            };
        }

        virtual R ranking(Move mv, const Board& board) = 0;

        bool isDeterministic() const override {
            return false;
        }
    };

    template<bool Ascending>
    using Ordering = std::conditional_t<Ascending, std::less<>, std::greater<>>;

    // TODO this technically does not need the move excursion (i.e. the board in post move state)
    //   but we pay the cost so maybe somehow be able to not use that?
    //   in fact PGN based wants the original board...
    template<bool Ascending = true, bool FromFirst = true>
    class LexicographicallyPlayer : public RankingPlayer<
                                            std::pair<BoardIndex, BoardIndex>,
                                            Ordering<Ascending>> {
    public:
        std::pair<BoardIndex, BoardIndex> rankMove(Move mv, const Board&) final {
            if constexpr (FromFirst) {
                return std::make_pair(BoardIndex{mv.fromPosition}, BoardIndex{mv.toPosition});
            }
            return std::make_pair(BoardIndex{mv.toPosition}, BoardIndex{mv.fromPosition});
        }

        [[nodiscard]] std::string name() const override {
            std::string name = "Lexicographically";
            if constexpr (Ascending) {
                name += "First";
            } else {
                name += "Last";
            }
            if constexpr (FromFirst) {
                name += "FromFirst";
            } else {
                name += "ToFirst";
            }
            return name;
        }
    };

    template<bool Ascending = true>
    class PGNAlphabeticallyPlayer : public RankingPlayer<std::string, Ordering<Ascending>> {
    public:
        std::string name() const override {
            return std::string("PGN") + (Ascending ? "AFirst" : "ZFirst");
        }
        std::string rankMove(Move mv, const Board& board) override {
            return board.moveToSAN(mv);
        }
    };


    class IndexPlayer : public StatelessPlayer {
    public:
        Move pickMove(const Board& board, const MoveList& list) final;

        virtual size_t index(const Board& board, const MoveList& list) = 0;

        bool isDeterministic() const override;
    };

    class RandomPlayer : public IndexPlayer {
    public:
        size_t index(const Board& board, const MoveList& list) override;

        [[nodiscard]] std::string name() const override;
    };

    class ConstIndexPlayer : public IndexPlayer {
    public:
        explicit ConstIndexPlayer(int32_t i) : val(i) {};

        size_t index(const Board& board, const MoveList& list) override;

        [[nodiscard]] std::string name() const override;
    private:
        int32_t val;
    };

    template<bool Least>
    class CountOpponentMoves : public EvaluateAfterMovePlayer<int32_t, Ordering<Least>> {
    public:
        int32_t ranking(Move, const Board &board) final {
            MoveList list = generateAllMoves(board);
            if (list.size() > 0) {
                return int32_t(list.size());
            }

            // prefer checkmate over stalemate
            if (list.isCheckMate()) {
                return -1;
            }
            // This does still prefer stale mate over moves left but that is the point
            return 0;
        }

        std::string name() const final {
            return (Least ? "Least" : "Most") + std::string(" opponent moves");
        }
    };

    class ProgressiveIndexPlayer : public Player {
    public:
        struct ProgressiveIndexPlayerState : public PlayerGameState {
            explicit ProgressiveIndexPlayerState(const std::function<int32_t(int32_t)>& operation, Color us, int32_t val);

            Move pickMove(const Board& board, const MoveList& list) override;

            void movePlayed(Move move, const Board& board) override;
        private:
            int32_t val = 0;
            const std::function<int32_t(int32_t)>& operation;
            Color me;
        };

        std::unique_ptr<PlayerGameState> startGame(Color color) const override;
        std::string name() const override;
        bool isDeterministic() const override {
            return true;
        }

        ProgressiveIndexPlayer(std::string baseName, std::function<int32_t(int32_t)> mOperation, int32_t mStartVal = 0);

    private:
        std::string baseName;
        int32_t m_startVal;
        std::function<int32_t(int32_t)> m_operation;
    };

    using LeastOpponentMoves = CountOpponentMoves<true>;
    using MostOpponentMoves = CountOpponentMoves<false>;

    std::unique_ptr<Player> randomPlayer();

    std::unique_ptr<Player> indexPlayer(int32_t val);

    std::unique_ptr<Player> minOpponentMoves();

    std::unique_ptr<Player> maxOpponentMoves();

    std::unique_ptr<Player> lexicographically(bool ascending = true, bool from = true);

    std::unique_ptr<Player> alphabetically(bool ascending = true);

    std::unique_ptr<Player> indexOp();

}
