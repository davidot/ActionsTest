#pragma once

#include "Types.h"
#include "Move.h"
#include "Piece.h"
#include <array>
#include <cstdint>
#include <deque>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace Chess {
    class MoveList;
    struct ExpectedBoard;

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

        static_assert(sizeof(BitBoard) * 8 == size * size);

        bool operator==(const Board &rhs) const;

        [[nodiscard]] std::string moveToSAN(Move, const MoveList&) const;

        [[nodiscard]] std::string moveToSAN(Move mv) const;

        [[nodiscard]] std::optional<Move> parseSANMove(std::string_view, const MoveList&) const;

        [[nodiscard]] std::optional<Move> parseSANMove(std::string_view) const;

        // Note: does not check stalemate
        [[nodiscard]] bool isDrawn(bool forced = false) const;

        // Note: counts null move as irreversible move
        [[nodiscard]] uint32_t positionRepeated() const;

        template<typename F>
        auto moveExcursion(Move mv, F&& func) const {
            auto& me = const_cast<Board&>(*this);
            me.makeMove(mv);
            const Board& constMe = *this;
            if constexpr (std::is_void_v<std::invoke_result_t<F, const Board&>>) {
                func(constMe);
                me.undoMove();
            } else {
                auto r = func(constMe);
                me.undoMove();
                return r;
            }
        }

        // Must! be a pseudo legal (does not check this and could fail)
        [[nodiscard]] bool isLegal(Move) const;

        [[nodiscard]] bool attacked(BoardIndex col, BoardIndex row) const;

        // TODO: isPseudoLegal
    private:
        std::optional<std::string> parseFENBoard(std::string_view);

        std::optional<std::string> setAvailableCastles(std::string_view vw);

        [[nodiscard]] std::optional<Piece> pieceAt(BoardIndex index) const;

        void setPiece(BoardIndex index, std::optional<Piece> piece);

        [[nodiscard]] static BoardIndex columnRowToIndex(BoardIndex column, BoardIndex row);

        [[nodiscard]] static std::pair<BoardIndex, BoardIndex> indexToColumnRow(BoardIndex);

        [[nodiscard]] static std::string indexToSAN(BoardIndex);

        [[nodiscard]] static std::optional<BoardIndex> SANToIndex(std::string_view);

        [[nodiscard]] uint32_t findRepetitions() const;

        [[nodiscard]] bool attacked(BoardIndex index) const;

        std::array<Piece::IntType, size * size> m_pieces;

        Color m_nextTurnColor = Color::White;
        CastlingRight m_castlingRights = CastlingRight::NoCastling;
        std::optional<BoardIndex> m_enPassant = std::nullopt;

        uint32_t m_halfMovesMade = 0;
        uint32_t m_halfMovesSinceCaptureOrPawn = 0;

        uint32_t m_repeated = 0;

        struct MoveData {
            Move performedMove;
            std::optional<Piece> capturedPiece;
            std::optional<BoardIndex> previousEnPassant;
            CastlingRight previousCastlingRights = CastlingRight::NoCastling;
            uint32_t previousSinceCapture;
            uint32_t timesRepeated;

            MoveData(const Board& board, Move move);

            void takeValues(Board& board);
        };

        std::deque<MoveData> m_history;

#ifndef COMPUTE_KING_POS
#define STORE_KING_POS 1
#endif

#ifdef STORE_KING_POS
        constexpr static BoardIndex invalidVal = -1;
        std::array<BoardIndex, 2> m_kingPos = {invalidVal, invalidVal};
        static_assert(invalidVal > size, "-1 is used as out of bounds");
#endif

        // TODO: simplify the friend structure here
        friend struct Move;
        friend class MoveList;
        friend MoveList generateAllMoves(const Board& board);
        friend bool validateMove(MoveList& list, const Board&, Move);

        BitBoard piecesBB = 0u;
        std::array<BitBoard, 2> colorPiecesBB{};
        std::array<BitBoard, Piece::pieceTypes> typePiecesBB{};

        [[nodiscard]] BitBoard colorBitboard(Color) const;
        [[nodiscard]] BitBoard typeBitboard(Piece::Type) const;
        [[nodiscard]] std::optional<BitBoard> enPassantBB() const;

        [[nodiscard]] BitBoard attacksOn(BoardIndex index, BitBoard occupied) const;

        [[nodiscard]] BitBoard attacksOn(BoardIndex index) const {
            return attacksOn(index, piecesBB);
        };
        [[nodiscard]] BitBoard pieceBitBoard(Piece p) const;
        [[nodiscard]] BitBoard typeBitboards(Piece::Type tp1, Piece::Type tp2) const;

        [[nodiscard]] bool isPinned(BoardIndex square) const;
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
