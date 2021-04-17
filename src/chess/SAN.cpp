#include "Board.h"
#include "MoveGen.h"
#include "../util/Assertions.h"
#include <cctype>

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

    std::array<char, 8> sanPieceChar = {
            '~',
            ' ', // pawn has no type char
            'K',
            'B',
            'R',
            'Q',
            'N',
            '-'
    };

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


        std::string disambiguation = "";


        return typeChar(tp.type()) + disambiguation + destination;
    }

    std::optional<Move> Board::parseSANMove(std::string_view sv) const {
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
            return Move{from.value(), destination};
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

        switch (tp) {
            case Piece::Type::Pawn:
                if (capturing || destination == m_enPassant) {
                    ASSERT(fromCol < size);
                    BoardIndex row = toRow - pawnDirection(colorToMove());

                    if (destination == m_enPassant) {
                        ASSERT(!pieceAt(destination).has_value());
                        ASSERT(flag == Move::Flag::None);
                        flag = Move::Flag::EnPassant;
                    }

                    return Move{columnRowToIndex(fromCol, row), destination, flag};
                } else {
                    ASSERT(sv.empty());
                    Color us = colorToMove();
                    BoardOffset pawnDir = pawnDirection(us);
                    if (pieceAt(toCol, toRow - pawnDir) == Piece{Piece::Type::Pawn, us}) {
                        return Move(toCol, toRow - pawnDir, toCol, toRow, flag);
                    }
                    // must be double push
                    ASSERT(flag == Move::Flag::None);
                    if (pieceAt(toCol, toRow - pawnDir - pawnDir) == Piece{Piece::Type::Pawn, us}) {
                        return Move(toCol, toRow - pawnDir - pawnDir, toCol, toRow, Move::Flag::DoublePushPawn);
                    }
                }
                ASSERT_NOT_REACHED();
                break;
            case Piece::Type::King:
                {
                    ASSERT(sv.empty());
                    // Should only be one king
                    auto [kingC, kingR] = kingSquare(colorToMove());
                    return Move{kingC, kingR, toCol, toRow};
                }
                break;
            case Piece::Type::Bishop:
                {
                    // TODO if disam figure out way to limit search
                    const Piece bishop{Piece::Type::Bishop, colorToMove()};
                    for (BoardIndex dCol : {1, -1}) {
                        for (BoardIndex dRow : {1, -1}) {
                            BoardIndex cCol = toCol + dCol;
                            BoardIndex cRow = toRow + dRow;
                            while (cCol < size && cRow < size) {
                                if (pieceAt(cCol, cRow) == bishop) {
                                    return Move{cCol, cRow, toCol, toRow};
                                }
                                cCol += dCol;
                                cRow += dRow;
                            }
                        }
                    }
                    ASSERT_NOT_REACHED();
                }
                break;
            case Piece::Type::Rook:
                {
                    const Piece rook{Piece::Type::Rook, colorToMove()};

                    for (BoardIndex dCol : {-1, 1}) {
                        BoardIndex cCol = toCol + dCol;
                        while (cCol < size) {
                            if (pieceAt(cCol, toRow) == rook) {
                                return Move{cCol, toRow, toCol, toRow};
                            }
                            cCol += dCol;
                        }
                    }

                    for (BoardIndex dRow : {-1, 1}) {
                        BoardIndex cRow = toRow + dRow;
                        while (cRow < size) {
                            if (pieceAt(toCol, cRow) == rook) {
                                return Move{toCol, cRow, toCol, toRow};
                            }
                            cRow += dRow;
                        }
                    }
                    ASSERT_NOT_REACHED();
                }
                break;
            case Piece::Type::Queen:
                {
                    // Diagonal
                    const Piece queen{Piece::Type::Queen, colorToMove()};
                    for (BoardIndex dCol : {1, -1}) {
                        for (BoardIndex dRow : {1, -1}) {
                            BoardIndex cCol = toCol + dCol;
                            BoardIndex cRow = toRow + dRow;
                            while (cCol < size && cRow < size) {
                                if (pieceAt(cCol, cRow) == queen) {
                                    return Move{cCol, cRow, toCol, toRow};
                                }
                                cCol += dCol;
                                cRow += dRow;
                            }
                        }
                    }

                    for (BoardIndex dCol : {-1, 1}) {
                        BoardIndex cCol = toCol + dCol;
                        while (cCol < size) {
                            if (pieceAt(cCol, toRow) == queen) {
                                return Move{cCol, toRow, toCol, toRow};
                            }
                            cCol += dCol;
                        }
                    }

                    for (BoardIndex dRow : {-1, 1}) {
                        BoardIndex cRow = toRow + dRow;
                        while (cRow < size) {
                            if (pieceAt(toCol, cRow) == queen) {
                                return Move{toCol, cRow, toCol, toRow};
                            }
                            cRow += dRow;
                        }
                    }

                    ASSERT_NOT_REACHED();
                }
                break;
            case Piece::Type::Knight:
                {
                    struct KO {
                        BoardOffset colOff;
                        BoardOffset rowOff;
                    };
                    // TODO: unify all knight offsets somewhere
                    constexpr std::array<KO, 8> knightOffsets = {{
                            {-2, -1},
                            {-1, -2},
                            {1, -2},
                            {2, -1},
                            {-2, 1},
                            {-1, 2},
                            {1, 2},
                            {2, 1}
                    }};
                    Piece knight{Piece::Type::Knight, colorToMove()};
                    for (KO ko : knightOffsets) {
                        BoardIndex fromCol = toCol - ko.colOff;
                        BoardIndex fromRow = toRow - ko.rowOff;
                        if (fromCol >= size || fromRow >= size) {
                            continue;
                        }
                        // TODO disam
                        if (pieceAt(fromCol, fromRow) == knight) {
                            return Move{fromCol, fromRow, toCol, toRow};
                        }
                    }
                    ASSERT_NOT_REACHED();
                }
                break;
            case Piece::Type::None:
                ASSERT_NOT_REACHED();
                break;
        }

        ASSERT_NOT_REACHED();
        return std::nullopt;
    }

}
