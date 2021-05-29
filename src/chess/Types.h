#pragma once

#include <cstdint>
#include <type_traits>

namespace Chess {
    using BoardIndex = uint8_t;
    using BoardOffset = std::make_signed_t<BoardIndex>;
    using BitBoard = uint64_t;
    class Board;
}