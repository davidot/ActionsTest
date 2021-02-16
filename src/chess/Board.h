#pragma once

#include "Piece.h"
#include <optional>
#include <string_view>
#include <vector>

namespace Chess {

    struct Move {
        // TODO: actually add interface
        uint16_t fromPosition;
        uint16_t toPosition;
        enum class Flags : uint8_t {
            None = 0,
        };
        uint32_t val;
    };

    class Board {
    public:
        [[nodiscard]] static Board fromFEN(std::string_view, bool extended = false);

        [[nodiscard]] static Board standardBoard();

        [[nodiscard]] static Board emptyBoard(uint8_t size = 8);

        bool makeMove(Move);

        bool undoMove(Move);

        [[nodiscard]] Piece::Color colorToMove() const;

        [[nodiscard]] bool hasValidPosition() const;

        [[nodiscard]] uint32_t countPieces(Piece::Color) const;

    private:
        Board(uint8_t size);

        uint8_t m_size;
        std::vector<Piece::IntType> m_pieces;

    };

}
