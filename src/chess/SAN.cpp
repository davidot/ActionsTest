#include "Board.h"
#include "Types.h"
#include "../util/Assertions.h"
#include "Move.h"
#include "MoveGen.h"
#include "Piece.h"
#include <array>
#include <cctype>
#include <optional>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

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

    constexpr static char firstRow = '1';


    BoardIndex letterToCol(char c) {
        ASSERT(c >= firstCol && c <= finalCol);
        return c - firstCol;
    }

    char colToLetter(BoardIndex col) {
        ASSERT(col < Board::size);
        return static_cast<char>(firstCol + col);
    }

    char rowToNumber(BoardIndex row) {
        ASSERT(row < Board::size);
        return static_cast<char>(firstRow + row);
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
        auto row = vw[1] - firstRow;
        return std::make_pair(col, row);
    }

    std::string Board::columnRowToSAN(BoardIndex col, BoardIndex row) {
        std::string str;
        str.push_back(colToLetter(col));
        str.push_back(static_cast<char>(firstRow + row));
        return str;
    }

    std::string Board::indexToSAN(BoardIndex index) {
        auto [col, row] = indexToColumnRow(index);
        return columnRowToSAN(col, row);
    }

    std::string Board::moveToSAN(Move mv) const {
        return moveToSAN(mv, generateAllMoves(*this));
    }

    constexpr std::array<char, 8> sanPieceChar = {
            '~',
            ' ', // pawn has no type char
            'K',
            'B',
            'R',
            'Q',
            'N',
            '-'
    };

    static_assert(sanPieceChar[static_cast<int>(Piece::Type::Rook)] == 'R');
    static_assert(sanPieceChar[static_cast<int>(Piece::Type::King)] == 'K');
    static_assert(sanPieceChar[static_cast<int>(Piece::Type::Queen)] == 'Q');

    Piece::Type parseTypeChar(char c) {
        switch (c) {
            case 'K':
                return Piece::Type::King;
            case 'B':
                return Piece::Type::Bishop;
            case 'R':
                return Piece::Type::Rook;
            case 'Q':
                return Piece::Type::Queen;
            case 'N':
                return Piece::Type::Knight;
            default:
                break;
        }
        return Piece::Type::Pawn;
    }

    char typeChar(Piece::Type tp) {
        return sanPieceChar[static_cast<uint8_t>(tp)];
    }

    std::string Board::moveToSAN(Move mv, const MoveList& list) const {
        ASSERT(list.contains(mv));
        ASSERT(pieceAt(mv.fromPosition).has_value());

        if (mv.flag == Move::Flag::Castling) {
            auto [toCol, toRow] = mv.colRowToPosition();
            ASSERT(toRow == homeRow(colorToMove()));
            if (toCol > kingCol) {
                ASSERT(toCol == kingSideRookCol);
                return "O-O";
            }
            ASSERT(toCol == queenSideRookCol);
            return "O-O-O";
        }


        Piece tp = *pieceAt(mv.fromPosition);
        std::string destination = indexToSAN(mv.toPosition);

        bool capturing = pieceAt(mv.toPosition).has_value();
        ASSERT(!capturing || pieceAt(mv.toPosition)->color() != colorToMove());

        if (tp.type() == Piece::Type::Pawn) {
            if (mv.isPromotion()) {
                destination.push_back('=');
                destination.push_back(Piece{mv.promotedType(), Color::White}.toFEN());
            }

            if (capturing || mv.flag == Move::Flag::EnPassant) {
                ASSERT(capturing || mv.toPosition == m_enPassant);
                auto [fromCol, fromRow] = mv.colRowFromPosition();

                return colToLetter(fromCol) + ('x' + destination);
            }
            return destination;
        }

        if (capturing) {
            destination.insert(0, "x");
        }

        if (tp.type() == Piece::Type::King) {
            return typeChar(tp.type()) + destination;
        }

        // determine location(s) of pieces of same type which could move there as well

        bool multiple = false;
        bool colAmbiguous = false;
        bool rowAmbiguous = false;

        auto [fromCol, fromRow] = mv.colRowFromPosition();

        list.forEachFilteredMove(
                [toPos = mv.toPosition, fromPos = mv.fromPosition](const Move& move) {
                    return move.toPosition == toPos && move.fromPosition != fromPos;
                },
                 [&, fromCol = fromCol, fromRow = fromRow](const Move& move) {
                         ASSERT(pieceAt(move.fromPosition).has_value() && pieceAt(move.fromPosition)->color() == colorToMove());
                         if (pieceAt(move.fromPosition) == tp) {
                             // ambiguity possible
                             multiple = true;
                             ASSERT(move.fromPosition != mv.fromPosition);
                             auto [fromC, fromR] = move.colRowFromPosition();

                             if (fromC == fromCol) {
                                 colAmbiguous = true;
                             }
                             if (fromR == fromRow) {
                                 rowAmbiguous = true;
                             }
                         }
                     });

        std::string disambiguation = "";

        if (multiple) {
            if (colAmbiguous && rowAmbiguous) {
                disambiguation = indexToSAN(mv.fromPosition);
            } else if (colAmbiguous && !rowAmbiguous) {
                disambiguation = rowToNumber(fromRow);
            } else {
                disambiguation = colToLetter(fromCol);
            }
        }


        return typeChar(tp.type()) + disambiguation + destination;
    }

    std::optional<Move> Board::parseSANMove(std::string_view sv) const {
        return parseSANMove(sv, generateAllMoves(*this));
    }

    std::optional<Move> Board::parseSANMove(std::string_view sv, const MoveList& moves) const {
        ASSERT(sv.size() >= 2);
        // do not want the check or checkmate data
        ASSERT(sv.back() != '+' && sv.back() != '#');
        Move::Flag flag = Move::Flag::None;

        if (sv[0] == 'O') {
            // must be castling
            ASSERT(sv[1] == '-' && sv[2] == 'O');
            BoardIndex home = Board::homeRow(colorToMove());
            if (sv.size() == 3) {
                return Move{kingCol, home, kingSideRookCol, home, Move::Flag::Castling};
            } else {
                ASSERT(sv.size() == 5);
                ASSERT(sv[3] == '-' && sv[4] == 'O');
                return Move{kingCol, home, queenSideRookCol, home, Move::Flag::Castling};
            }
        }

        if (sv[sv.size() - 2] == '=') {
            ASSERT(sv.size() >= 4);
            // it is a promotion
            Piece::Type tp = parseTypeChar(sv[sv.size() - 1]);
            switch (tp) {
                case Piece::Type::Queen:
                    flag = Move::Flag::PromotionToQueen;
                    break;
                case Piece::Type::Knight:
                    flag = Move::Flag::PromotionToKnight;
                    break;
                case Piece::Type::Bishop:
                    flag = Move::Flag::PromotionToBishop;
                    break;
                case Piece::Type::Rook:
                    flag = Move::Flag::PromotionToRook;
                    break;
                default:
                    ASSERT_NOT_REACHED();
            }
            sv.remove_suffix(2);
        }

        // TODO: should these be ifs? or have some "fast" path which may return some move on invalid input
        auto optDest = SANToColRow(sv.substr(sv.size() - 2, 2));
        if (!optDest.has_value()) {
            return std::nullopt;
        }
        auto [toCol, toRow] = optDest.value();
        BoardIndex destination = columnRowToIndex(toCol, toRow);
        sv.remove_suffix(2);

        bool capturing = !sv.empty() && sv.back() == 'x';
        if (capturing) {
            sv.remove_suffix(1);
        }

        Piece::Type tp = sv.empty() ? Piece::Type::Pawn : parseTypeChar(sv.front());
        if (tp != Piece::Type::Pawn) {
            sv.remove_prefix(1);
            ASSERT(flag == Move::Flag::None);
        }

        if (sv.size() == 2) {
            // fully disambiguated
            ASSERT(tp != Piece::Type::Pawn);
            auto from = SANToIndex(sv);
            ASSERT(from.has_value());
            Move move{from.value(), destination};
            ASSERT(moves.contains(move));
            return move;
        }

        BoardIndex fromCol = -1;
        BoardIndex fromRow = -1;
        if (!sv.empty()) {
            char c = sv.front();
            if (std::isdigit(c)) {
                fromRow = c - firstRow;
            } else {
                ASSERT(std::islower(c));
                fromCol = letterToCol(c);
            }
        }

        Color us = colorToMove();

        if (tp == Piece::Type::Pawn) {
            Move mv;
            if (capturing || destination == m_enPassant) {
                ASSERT(fromCol < size);
                BoardIndex row = toRow - pawnDirection(colorToMove());

                if (destination == m_enPassant) {
                    ASSERT(!pieceAt(destination).has_value());
                    ASSERT(flag == Move::Flag::None);
                    flag = Move::Flag::EnPassant;
                }

                mv = Move{columnRowToIndex(fromCol, row), destination, flag};
            } else {
                ASSERT(sv.empty());
                Piece pawn{Piece::Type::Pawn, us};

                BoardOffset pawnDir = pawnDirection(us);
                if (pieceAt(toCol, toRow - pawnDir) == pawn) {
                    mv = Move(toCol, toRow - pawnDir, toCol, toRow, flag);
                } else {
                    // must be double push so cannot be promotion or e.p.
                    ASSERT(flag == Move::Flag::None);
                    ASSERT(pieceAt(toCol, toRow - pawnDir - pawnDir) == pawn);
                    mv = Move(toCol, toRow - pawnDir - pawnDir, toCol, toRow, Move::Flag::DoublePushPawn);
                }
            }
            ASSERT(moves.contains(mv));
            return mv;
        }

        bool foundSingleMove = false;
        Move mv;

        moves.forEachFilteredMove([&foundSingleMove, destination](const Move& move) {
            return !foundSingleMove && move.toPosition == destination;
        },
                                  [&](const Move& move) {
                                      ASSERT(pieceAt(move.fromPosition).has_value() && pieceAt(move.fromPosition)->color() == us);
                                      auto [fromC, fromR] = move.colRowFromPosition();
                                      if ((fromCol < size && fromC != fromCol)
                                          || (fromRow < size && fromR != fromRow)) {
                                          return;
                                      }
                                      if (pieceAt(move.fromPosition)->type() == tp) {
                                          foundSingleMove = true;
                                          mv = move;
                                      }
                                  });
        if (!foundSingleMove) {
            return std::nullopt;
        }
        return mv;
    }

}
