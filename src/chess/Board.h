#pragma once

#include "Piece.h"
#include <optional>
#include <string_view>
#include <variant>
#include <vector>
#include <array>
#include "Forward.h"

namespace Chess {

    enum class CastlingRight : uint8_t {
        NoCastling = 0u,
        WhiteKingSide = 1u,
        WhiteQueenSide = WhiteKingSide << 1,
        BlackKingSide = 1u << 2,
        BlackQueenSide = BlackKingSide << 1,

        WhiteCastling = WhiteKingSide | WhiteQueenSide,
        BlackCastling = BlackKingSide | BlackQueenSide,
        KingSideCastling = WhiteKingSide | BlackKingSide,
        QueenSideCastling = WhiteQueenSide | BlackQueenSide,
        AnyCastling = WhiteCastling | BlackCastling
    };

    class Board {
    public:
        using BoardIndex = uint8_t;

        [[nodiscard]] static ExpectedBoard fromFEN(std::string_view);

        [[nodiscard]] static Board standardBoard();

        [[nodiscard]] static Board emptyBoard();

        bool makeMove(Move);

        bool undoMove(Move);

        [[nodiscard]] Color colorToMove() const;

        [[nodiscard]] bool hasValidPosition() const;

        [[nodiscard]] uint32_t countPieces(Color) const;

        [[nodiscard]] std::optional<Piece> pieceAt(std::string_view) const;

        [[nodiscard]] std::optional<Piece> pieceAt(BoardIndex column, BoardIndex row) const;

        [[nodiscard]] std::optional<Piece> pieceAt(std::pair<BoardIndex, BoardIndex> coords) const;

        void setPiece(std::string_view, std::optional<Piece> piece);

        void setPiece(BoardIndex column, BoardIndex row, std::optional<Piece> piece);

        [[nodiscard]] uint8_t size() const;

        [[nodiscard]] std::string toFEN() const;

        [[nodiscard]] static std::string columnRowToSAN(BoardIndex column, BoardIndex row);

        [[nodiscard]] static std::optional<std::pair<BoardIndex, BoardIndex>> SANToColRow(std::string_view);

        void makeNullMove();

        void undoNullMove();

        [[nodiscard]] std::optional<std::pair<Board::BoardIndex, Board::BoardIndex>> enPassantColRow() const;

        [[nodiscard]] CastlingRight castlingRights() const;

        std::pair<Board::BoardIndex, Board::BoardIndex> kingSquare(Color color) const;

    private:
        std::optional<std::string> parseFENBoard(std::string_view);

        bool setAvailableCastles(std::string_view vw);

        [[nodiscard]] std::optional<Piece> pieceAt(BoardIndex index) const;

        void setPiece(BoardIndex index, std::optional<Piece> piece);

        [[nodiscard]] static BoardIndex columnRowToIndex(BoardIndex column, BoardIndex row);

        [[nodiscard]] static std::pair<BoardIndex, BoardIndex> indexToColumnRow(BoardIndex);

        [[nodiscard]] static std::string indexToSAN(BoardIndex);

        [[nodiscard]] static std::optional<BoardIndex> SANToIndex(std::string_view);

        static constexpr const BoardIndex m_size = 8;
        std::array<Piece::IntType, m_size * m_size> m_pieces;
        std::array<uint8_t, 2> m_numPieces = {0, 0};
        Color m_nextTurnColor = Color::White;
        CastlingRight m_castlingRights = CastlingRight::NoCastling;
        std::optional<BoardIndex> m_enPassant = std::nullopt;
        uint32_t m_fullMoveNum = 1;
        uint32_t m_halfMovesSinceCaptureOrPawn = 0;

        friend struct Move;
        friend class MoveList;
    };

    // TODO to make this actually fit in 16 bits use: struct __attribute__((packed)) Move {
    struct Move {
        using BoardOffset = std::make_signed_t<Board::BoardIndex>;
        enum class Flag : uint8_t {
            None = 0,
            Castling = 1,
            DoublePushPawn = 2,
            EnPassant = 3,
            PromotionToKnight = 4,
            PromotionToBishop = 5,
            PromotionToRook = 6,
            PromotionToQueen = 7
        };

        Board::BoardIndex toPosition: 6;
        Board::BoardIndex fromPosition : 6;

        Flag flag : 3;

        Move();

        Move(Board::BoardIndex fromPosition, Board::BoardIndex toPosition, Flag flags = Flag::None);

        Move(Board::BoardIndex fromCol, Board::BoardIndex fromRow, BoardOffset offset, Flag flags = Flag::None);

        Move(Board::BoardIndex fromCol, Board::BoardIndex fromRow,
             Board::BoardIndex toCol, Board::BoardIndex toRow, Flag flags = Flag::None);


        std::pair<Board::BoardIndex, Board::BoardIndex> colRowFromPosition() const;
        std::pair<Board::BoardIndex, Board::BoardIndex> colRowToPosition() const;

        bool isPromotion() const;

        Piece::Type promotedType() const;
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
