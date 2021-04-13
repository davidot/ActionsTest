#pragma once

#include <cstdint>

namespace Chess {
    using BoardIndex = uint8_t;
    using BoardOffset = std::make_signed_t<BoardIndex>;


    struct ExpectedBoard;
    struct Move;
    class Board;
    class MoveList;
}