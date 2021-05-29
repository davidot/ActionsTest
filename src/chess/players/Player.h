#pragma once

#include "../Board.h"
#include "../Move.h"
#include <memory>
#include <vector>
#include <string>

namespace Chess {

    class Explainer {
    public:
        virtual ~Explainer() = default;

        virtual bool enabled() = 0;

        explicit operator bool() {
            return enabled();
        }

        struct ExplainedMove {
            Move mv;
            int32_t score;
            std::string comment;
        };

        virtual void outputMoveListRanking(std::vector<ExplainedMove>) = 0;

    };

    class NullExplainer : Explainer {
        bool enabled() override {
            return false;
        }
    };


    class PlayerGameState {
    public:
        virtual ~PlayerGameState() = default;

        virtual Move pickMove(const Board& board, const MoveList& list) = 0;

        virtual void movePlayed(Move move, const Board& board) {
        };
    };

    class Player {
    public:
        virtual ~Player() = default;

        [[nodiscard]] virtual std::unique_ptr<PlayerGameState> startGame(Color) const = 0;

        [[nodiscard]] virtual std::string name() const = 0;

        [[nodiscard]] virtual bool isDeterministic() const = 0;
    };

    class StatelessPlayer {
    public:
        virtual ~StatelessPlayer() = default;

        [[nodiscard]] virtual Move pickMove(const Board& board, const MoveList& list) = 0;

        [[nodiscard]] virtual std::string name() const = 0;

        [[nodiscard]] virtual bool isDeterministic() const = 0;
    };


    class StatelessState : public PlayerGameState {
    public:
        Move pickMove(const Board& board, const MoveList& list) override {
            return m_plr.pickMove(board, list);
        }

        explicit StatelessState(StatelessPlayer& plr) : m_plr(plr) {
        }

    private:
        StatelessPlayer& m_plr;
    };

    template<typename P>
    class StatelessWrapper final : public Player {
    public:
        [[nodiscard]] std::unique_ptr<PlayerGameState> startGame(Color) const final {
            return std::make_unique<StatelessState>(*m_player);
        }

        [[nodiscard]] std::string name() const final {
            return m_player->name();
        }

        [[nodiscard]] bool isDeterministic() const override {
            return m_player->isDeterministic();
        }

        template<typename ...Args>
        explicit StatelessWrapper(Args... args) : m_player(std::make_unique<P>(std::forward<Args>(args)...)) {
        }
    private:
        std::unique_ptr<StatelessPlayer> m_player;
    };


    template<typename T, typename ...Args>
    static std::unique_ptr<Player> make_stateless(Args... args) {
        return std::make_unique<StatelessWrapper<T>>(std::forward<Args>(args)...);
    }


}
