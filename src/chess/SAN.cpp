#include "Board.h"
#include "MoveGen.h"
#include "../util/Assertions.h"

namespace Chess {

    std::optional<BoardIndex> Board::SANToIndex(std::string_view vw) {
        auto colRow = SANToColRow(vw);
        if (!colRow.has_value()) {
            return std::nullopt;
        }
        return columnRowToIndex(colRow->first, colRow->second);
    }

    constexpr static char firstCol = 'a';
    constexpr static char finalCol = 'h';

    BoardIndex letterToCol(char c) {
        ASSERT(c >= firstCol && c <= finalCol);
        return c - firstCol;
    }

    char colToLetter(BoardIndex col) {
        ASSERT(col < Board::size);
        return static_cast<char>(firstCol + col);
    }

    std::optional<std::pair<BoardIndex, BoardIndex>> Board::SANToColRow(std::string_view vw) {
        if (vw.size() != 2) {
            return std::nullopt;
        }

        if (!std::islower(vw[0]) || !std::isdigit(vw[1])) {
            return std::nullopt;
        }

        if (vw[0] > 'h' || vw[1] == '9' || vw[1] == '0') {
            return std::nullopt;
        }

        auto col = letterToCol(vw[0]);
        auto row = vw[1] - '1';
        return std::make_pair(col, row);
    }

    std::string Board::columnRowToSAN(BoardIndex col, BoardIndex row) {
        std::string str;
        str.push_back(colToLetter(col));
        str.push_back('1' + row);
        return str;
    }

    std::string Board::indexToSAN(BoardIndex index) {
        auto [col, row] = indexToColumnRow(index);
        return columnRowToSAN(col, row);
    }

    std::string Board::moveToSAN(Move mv) const {
        return moveToSAN(mv, generateAllMoves(*this));
    }

    std::string Board::moveToSAN(Move mv, const MoveList& list) const {
        ASSERT(list.contains(mv));
        return indexToSAN(mv.toPosition);
    }

    std::optional<Move> Board::parseSANMove(std::string_view sv) const {
        auto pos = SANToColRow(sv);
        if (!pos) {
            return std::nullopt;
        }
        return Move(pos->first, pos->second - 1u, pos->first, pos->second);
    }

}
