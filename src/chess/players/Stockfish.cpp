#include "Stockfish.h"
#include <iostream>

#include "../../util/Assertions.h"
#include "../../util/Process.h"
#include "../../util/StringUtil.h"
#include "../Board.h"

//#define STOCKFISH_DEBUG
#ifdef STOCKFISH_DEBUG
#define READ_LINE(line) std::cout << "Stockfish -> " << line

#define WRITE_LINE(line) std::cout << "Stockfish <- " << line
#else
#define READ_LINE(line)
#define WRITE_LINE(line)
#endif

namespace Chess {

    std::string Stockfish::SearchLimit::toLimit() const {
        std::string base;
        switch (type) {
            case Nodes:
                base = "nodes";
                break;
            case MoveTime:
                base = "movetime";
                break;
            case Depth:
                base = "depth";
                break;
        }
        return base + ' ' + std::to_string(val);
    }

    Stockfish::SearchLimit Stockfish::SearchLimit::nodes(uint32_t nodes) {
        return {LimitType::Nodes, nodes};
    }

    Stockfish::SearchLimit Stockfish::SearchLimit::moveTime(uint32_t time) {
        return {LimitType::MoveTime, time};
    }

    Stockfish::SearchLimit Stockfish::SearchLimit::depth(uint32_t depth) {
        return {LimitType::Depth, depth};
    }

    Stockfish::SearchLimit::SearchLimit(Stockfish::SearchLimit::LimitType tp, uint32_t val) :
            val(val),
            type(tp) {
    }

    Stockfish::MoveResult Stockfish::bestMove(const Board& board) const {
        WRITE_LINE("position fen " + board.toFEN() + "\n" + m_limitedGo);
        m_proc->writeTo("position fen " + board.toFEN() + "\n" + m_limitedGo);
        std::string line;

        std::string lastInfo;
        while (m_proc->readLine(line)) {
            READ_LINE(line);
            if (line.find("seldepth") != std::string::npos) {
                lastInfo = line;
            } else if (line.find("bestmove") != std::string::npos) {
                break;
            }
        }

        ASSERT(line.find("bestmove") != std::string::npos);
        ASSERT(line.back() == '\n');
        line.pop_back();

        auto parts = util::split(line, " ");
        ASSERT(parts.size() >= 2);
        auto bestMove = parts[1];

        // TODO: extract score
//        std::cout << "Got move: " << bestMove << " and score: \n" << lastInfo;

        std::cout << "Bestmove line: " << line << '\n';

        return {
            std::string(bestMove),
        };
    }

    static constexpr const char* STOCKFISH_LOCATION = "stockfish.exe";

    Stockfish::Stockfish(Stockfish::SearchLimit limit, int difficulty) {
        m_proc = util::SubProcess::create({STOCKFISH_LOCATION});
        // TODO: maybe wrap in factory to ensure it started properly?
        ASSERT(m_proc);

        WRITE_LINE("uci\n");
        m_proc->writeTo("uci\n");
        std::string line;
        while (m_proc->readLine(line)) {
            READ_LINE("Start up _" << line << "_\n");
            if (line.find("uciok") != std::string::npos) {
                break;
            }
        }

        if (difficulty < 20 && difficulty >= 0) {
            WRITE_LINE("setoption name Skill Level value " + std::to_string(difficulty) + "\n");
            m_proc->writeTo("setoption name Skill Level value " + std::to_string(difficulty) + "\n");
        }

        m_limitedGo = "go " + limit.toLimit() + "\n";
    }

    Stockfish::~Stockfish() = default;

}
