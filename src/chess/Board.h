#pragma once

#include "Piece.h"
#include <optional>
#include <string_view>
#include <variant>
#include <vector>
#include <array>

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

    struct ExpectedBoard;

    enum class CastlingRight : uint8_t {
        NO_CASTLING = 0u,
        WHITE_KING_SIDE = 1u,
        WHITE_QUEEN_SIDE = WHITE_KING_SIDE << 1,
        BLACK_KING_SIDE = WHITE_KING_SIDE << 2,
        BLACK_QUEEN_SIDE = WHITE_KING_SIDE << 3,

        WHITE_CASTLING = WHITE_KING_SIDE | WHITE_QUEEN_SIDE,
        BLACK_CASTLING = BLACK_KING_SIDE | BLACK_QUEEN_SIDE,
        ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING
    };

    class Board {
    public:
        [[nodiscard]] static ExpectedBoard fromFEN(std::string_view);

        [[nodiscard]] static ExpectedBoard fromExtendedFEN(std::string_view);

        [[nodiscard]] static Board standardBoard();

        [[nodiscard]] static Board emptyBoard(uint8_t size = 8);

        bool makeMove(Move);

        bool undoMove(Move);

        [[nodiscard]] Color colorToMove() const;

        [[nodiscard]] bool hasValidPosition() const;

        [[nodiscard]] uint32_t countPieces(Color) const;

        std::optional<Piece> pieceAt(std::string_view) const;

        std::optional<Piece> pieceAt(uint8_t column, uint8_t row) const;

        std::optional<Piece> pieceAt(uint16_t index) const;

        void setPiece(std::string_view, std::optional<Piece> piece);

        void setPiece(uint8_t column, uint8_t row, std::optional<Piece> piece);

        void setPiece(uint16_t index, std::optional<Piece> piece);

        [[nodiscard]] uint8_t size() const;

        [[nodiscard]] std::string toFEN() const;

    private:
        explicit Board(uint8_t size);

        uint8_t m_size;
        std::vector<Piece::IntType> m_pieces;

        std::array<uint16_t, 2> m_numPieces = {0, 0};

        Color m_next_turn = Color::White;

        std::optional<std::string> parseFENBoard(std::string_view);

        [[nodiscard]] uint16_t columnRowToIndex(uint8_t column, uint8_t row) const;

        [[nodiscard]] std::pair<uint8_t, uint8_t> indexToColumnRow(uint16_t) const;

        [[nodiscard]] std::optional<uint16_t> SANToIndex(std::string_view) const;

        [[nodiscard]] std::string indexToSAN(uint16_t) const;

        bool setAvailableCastles(std::string_view vw);

        CastlingRight m_castlingRights = CastlingRight::NO_CASTLING;

        std::optional<uint16_t> m_enPassant = std::nullopt;

        uint32_t m_fullMoveNum = 1;

        uint32_t m_halfMovesSinceCaptureOrPawn = 0;
    };

    CastlingRight& operator|=(CastlingRight& lhs, const CastlingRight& rhs);
    CastlingRight operator&(const CastlingRight& lhs, const CastlingRight& rhs);

    struct ExpectedBoard {
    private:
        using T = Chess::Board;
        std::variant<T, std::string> m_value;

    public:
        ExpectedBoard(T&& val) : m_value(std::forward<T&&>(val)) {
        }

        ExpectedBoard(std::string error) : m_value(std::move(error)) {
        }

        ExpectedBoard(const char* error) : m_value(error) {
        }

        explicit operator bool() const {
            return m_value.index() == 0;
        }

        [[nodiscard]] const std::string& error() const{
            //ASSERT
            // in case it is a string we need to use the indices
            return std::get<1>(m_value);
        }

        [[nodiscard]] T&& extract() {
            // in case it is a string we need to use the indices
            return std::move(std::get<0>(m_value));
        }

        [[nodiscard]] const T& value() {
            // in case it is a string we need to use the indices
            return std::get<0>(m_value);
        }
    };

}
