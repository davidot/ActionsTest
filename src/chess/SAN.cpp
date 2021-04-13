#include "Board.h"

namespace Chess {

    std::optional<BoardIndex> Board::SANToIndex(std::string_view vw) {
        if (vw.size() != 2) {
            return std::nullopt;
        }

        if (!std::islower(vw[0]) || !std::isdigit(vw[1])) {
            return std::nullopt;
        }

        if (vw[0] > 'h' || vw[1] == '9' || vw[1] == '0') {
            return std::nullopt;
        }

        auto col = vw[0] - 'a';
        auto row = vw[1] - '1';

        return columnRowToIndex(col, row);
    }

    std::optional<std::pair<BoardIndex, BoardIndex>> Board::SANToColRow(std::string_view vw) {
        auto pos = SANToIndex(vw);
        if (pos) {
            return indexToColumnRow(pos.value());
        }
        return std::nullopt;
    }

    std::string Board::columnRowToSAN(BoardIndex col, BoardIndex row) {
        std::string str;
        str.push_back('a' + col);
        str.push_back('1' + row);
        return str;
    }

    std::string Board::indexToSAN(BoardIndex index) {
        auto [col, row] = indexToColumnRow(index);
        return columnRowToSAN(col, row);
    }

}
