#pragma once

#include "Piece.h"
#include <optional>
#include <string_view>
#include <variant>
#include <vector>
#include <array>

namespace Chess {

    struct ExpectedBoard;
    struct Move;

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
        using BoardIndex = uint8_t;

        [[nodiscard]] static ExpectedBoard fromFEN(std::string_view);

        [[nodiscard]] static ExpectedBoard fromExtendedFEN(std::string_view);

        [[nodiscard]] static Board standardBoard();

        [[nodiscard]] static Board emptyBoard();

        bool makeMove(Move);

        bool undoMove(Move);

        [[nodiscard]] Color colorToMove() const;

        [[nodiscard]] bool hasValidPosition() const;

        [[nodiscard]] uint32_t countPieces(Color) const;

        std::optional<Piece> pieceAt(std::string_view) const;

        std::optional<Piece> pieceAt(BoardIndex column, BoardIndex row) const;

        std::optional<Piece> pieceAt(BoardIndex index) const;

        void setPiece(std::string_view, std::optional<Piece> piece);

        void setPiece(BoardIndex column, BoardIndex row, std::optional<Piece> piece);

        void setPiece(BoardIndex index, std::optional<Piece> piece);

        [[nodiscard]] uint8_t size() const;

        [[nodiscard]] std::string toFEN() const;

        [[nodiscard]] static BoardIndex columnRowToIndex(uint8_t column, uint8_t row);

        [[nodiscard]] static std::pair<BoardIndex, BoardIndex> indexToColumnRow(BoardIndex);

    private:
        static constexpr const BoardIndex m_size = 8;
        std::array<Piece::IntType, m_size * m_size> m_pieces;

        std::array<uint8_t, 2> m_numPieces = {0, 0};

        Color m_next_turn = Color::White;

        std::optional<std::string> parseFENBoard(std::string_view);

        [[nodiscard]] std::optional<BoardIndex> SANToIndex(std::string_view) const;

        [[nodiscard]] std::string indexToSAN(uint16_t) const;

        bool setAvailableCastles(std::string_view vw);

        CastlingRight m_castlingRights = CastlingRight::NO_CASTLING;

        std::optional<BoardIndex> m_enPassant = std::nullopt;

        uint32_t m_fullMoveNum = 1;

        uint32_t m_halfMovesSinceCaptureOrPawn = 0;
    };

    // TODO to make this actually fit in 16 bits use: struct __attribute__((packed)) Move {
    struct Move {
        using BoardOffset = std::make_signed_t<Board::BoardIndex>;
        enum class Flags : uint8_t {
            None = 0,
            CastlingShort = 1,
            CastlingLong = 2,
            DoublePushPawn = 3,
            PromotionToKnight = 4,
            PromotionToBishop = 5,
            PromotionToRook = 6,
            PromotionToQueen = 7
        };

        Board::BoardIndex toPosition: 6;
        Board::BoardIndex fromPosition : 6;

        Flags flags : 3;

        Move();

        Move(Board::BoardIndex fromPosition, Board::BoardIndex toPosition, Flags flags = Flags::None);

        Move(Board::BoardIndex fromCol, Board::BoardIndex fromRow, BoardOffset offset, Flags flags = Flags::None);

        Move(Board::BoardIndex fromCol, Board::BoardIndex fromRow, Board::BoardIndex toCol, Board::BoardIndex toRow, Flags flags = Flags::None);

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

        [[nodiscard]] const std::string& error() const;

        [[nodiscard]] T&& extract();

        [[nodiscard]] const T& value();
    };

}
