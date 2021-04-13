#pragma once

#include "Forward.h"
#include "Move.h"
#include "Piece.h"
#include <array>
#include <deque>
#include <optional>
#include <string_view>
#include <variant>
#include <tuple>

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

    CastlingRight operator|(const CastlingRight& lhs, const CastlingRight& rhs);
    CastlingRight operator&(const CastlingRight& lhs, const CastlingRight& rhs);
    std::ostream& operator<<(std::ostream& strm, const CastlingRight& cr);

    class Board {
    public:
        [[nodiscard]] static ExpectedBoard fromFEN(std::string_view);

        [[nodiscard]] static Board standardBoard();

        [[nodiscard]] static Board emptyBoard();

        bool makeMove(Move);

        bool undoMove();

        [[nodiscard]] Color colorToMove() const;

        [[nodiscard]] bool hasValidPosition() const;

        [[nodiscard]] uint32_t countPieces(Color) const;

        [[nodiscard]] std::optional<Piece> pieceAt(std::string_view) const;

        [[nodiscard]] std::optional<Piece> pieceAt(BoardIndex column, BoardIndex row) const;

        [[nodiscard]] std::optional<Piece> pieceAt(std::pair<BoardIndex, BoardIndex> coords) const;

        void setPiece(std::string_view, std::optional<Piece> piece);

        void setPiece(BoardIndex column, BoardIndex row, std::optional<Piece> piece);

        [[nodiscard]] std::string toFEN() const;

        [[nodiscard]] static std::string columnRowToSAN(BoardIndex column, BoardIndex row);

        [[nodiscard]] static std::optional<std::pair<BoardIndex, BoardIndex>> SANToColRow(std::string_view);

        void makeNullMove();

        void undoNullMove();

        [[nodiscard]] std::optional<std::pair<BoardIndex, BoardIndex>> enPassantColRow() const;

        [[nodiscard]] CastlingRight castlingRights() const;

        [[nodiscard]] std::pair<BoardIndex, BoardIndex> kingSquare(Color color) const;

        [[nodiscard]] uint32_t fullMoves() const;

        [[nodiscard]] uint32_t halfMovesSinceIrreversible() const;

        // technically board specific chess constants
        constexpr static BoardIndex homeRow(Color color) {
            return color == Color::White ? 0 : 7;
        }

        constexpr static BoardOffset pawnDirection(Color color)  {
            return color == Color::White ? 1 : -1;
        }

        constexpr static BoardIndex pawnHomeRow(Color color)  {
            return homeRow(color) + pawnDirection(color);
        }

        constexpr static BoardIndex pawnPromotionRow(Color color) {
            return pawnHomeRow(opposite(color)) + pawnDirection(color);
        }

        constexpr static BoardIndex kingCol = 4;
        constexpr static BoardIndex queenSideRookCol = 0;
        constexpr static BoardIndex kingSideRookCol = 7;

        constexpr static BoardIndex size = 8;

        bool operator==(const Board &rhs) const;

    private:
        std::optional<std::string> parseFENBoard(std::string_view);

        std::optional<std::string> setAvailableCastles(std::string_view vw);

        [[nodiscard]] std::optional<Piece> pieceAt(BoardIndex index) const;

        void setPiece(BoardIndex index, std::optional<Piece> piece);

        [[nodiscard]] static BoardIndex columnRowToIndex(BoardIndex column, BoardIndex row);

        [[nodiscard]] static std::pair<BoardIndex, BoardIndex> indexToColumnRow(BoardIndex);

        [[nodiscard]] static std::string indexToSAN(BoardIndex);

        [[nodiscard]] static std::optional<BoardIndex> SANToIndex(std::string_view);

        std::array<Piece::IntType, size * size> m_pieces;

        Color m_nextTurnColor = Color::White;
        CastlingRight m_castlingRights = CastlingRight::NoCastling;
        std::optional<BoardIndex> m_enPassant = std::nullopt;

        uint32_t m_halfMovesMade = 0;
        uint32_t m_halfMovesSinceCaptureOrPawn = 0;


        struct MoveData {
            Move performedMove;
            std::optional<Piece> capturedPiece;
            std::optional<BoardIndex> previousEnPassant;
            CastlingRight previousCastlingRights = CastlingRight::NoCastling;
            uint32_t previousSinceCapture;

            MoveData(const Board& board, Move move);

            void takeValues(Board& board);
        };

        std::deque<MoveData> m_history;

        std::array<uint8_t, 2> m_numPieces = {0, 0};

#ifdef STORE_KING_POS
        constexpr static BoardIndex invalidVal = -1;
        std::array<BoardIndex, 2> m_kingPos = {invalidVal, invalidVal};
        static_assert(invalidVal > size, "-1 is used as out of bounds");
#endif

        friend struct Move;
        friend class MoveList;
    };

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
