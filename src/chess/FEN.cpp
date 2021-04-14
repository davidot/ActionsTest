#include "../util/Assertions.h"
#include "../util/StringUtil.h"
#include "Board.h"
#include "Piece.h"
#include <algorithm>
#include <charconv>
#include <optional>
#include <sstream>
#include <string_view>

namespace Chess {

    CastlingRight& operator|=(CastlingRight& lhs, const CastlingRight& rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    struct CastleMap {
        char c;
        CastlingRight right;

        CastleMap(Piece::Type tp, Color col, CastlingRight cr) noexcept :
            c(Piece(tp, col).toFEN()),
            right(cr) {
        }
    };

    const static std::array<CastleMap, 4> castleMapping = {
            CastleMap{Piece::Type::King, Color::White, CastlingRight::WhiteKingSide},
            CastleMap{Piece::Type::Queen, Color::White, CastlingRight::WhiteQueenSide},
            CastleMap{Piece::Type::King, Color::Black, CastlingRight::BlackKingSide},
            CastleMap{Piece::Type::Queen, Color::Black, CastlingRight::BlackQueenSide},
    };

    struct CastleCheck {
        CastlingRight right;
        BoardIndex col;
        Piece piece;
    };

    const static std::array<CastleCheck, 6> castleChecks = {
            CastleCheck{CastlingRight::WhiteCastling, Board::kingCol, Piece{Piece::Type::King, Color::White}},
            CastleCheck{CastlingRight::WhiteQueenSide, Board::queenSideRookCol, Piece{Piece::Type::Rook, Color::White}},
            CastleCheck{CastlingRight::WhiteKingSide, Board::kingSideRookCol, Piece{Piece::Type::Rook, Color::White}},
            CastleCheck{CastlingRight::BlackCastling, Board::kingCol, Piece{Piece::Type::King, Color::Black}},
            CastleCheck{CastlingRight::BlackQueenSide, Board::queenSideRookCol, Piece{Piece::Type::Rook, Color::Black}},
            CastleCheck{CastlingRight::BlackKingSide, Board::kingSideRookCol, Piece{Piece::Type::Rook, Color::Black}},
    };

    std::ostream& operator<<(std::ostream& stream, const CastlingRight& right) {
        bool any = false;
        for (auto& [fen, fenRight] : castleMapping) {
            if ((right & fenRight) != CastlingRight::NoCastling) {
                stream << fen;
                any = true;
            }
        }
        if (!any) {
            stream << '-';
        }
        return stream;
    }


    std::optional<std::string> Board::setAvailableCastles(std::string_view vw) {
        if (vw.empty() || vw.size() > 4) {
            return "Too many or few characters";
        }
        if (vw == "-") {
            return {};
        }
        //reset before hand
        m_castlingRights = CastlingRight::NoCastling;

        auto front = castleMapping.begin();

        for (const auto& c : vw) {
            if (auto foundMapping = std::find_if(front, castleMapping.end(), [&](CastleMap mapping) {
                  return mapping.c == c;
                });
                foundMapping == castleMapping.end()) {
                return std::string("Unknown character: ") + c;
            } else {
                m_castlingRights |= foundMapping->right;
                front = std::next(foundMapping);
            }
        }

        if (std::any_of(castleChecks.cbegin(), castleChecks.cend(), [&](const CastleCheck& check) {
          if ((m_castlingRights & check.right) != CastlingRight::NoCastling) {
              if (pieceAt(check.col, homeRow(check.piece.color())) != check.piece) {
                  return true;
              }
          }
          return false;
        })) {
            return std::string("Castling but pieces not present: ") + std::string(vw);
        }
        return {};
    }


    std::optional<std::string> Board::parseFENBoard(std::string_view view) {
        auto next = view.begin();

        uint32_t row = 7;
        uint32_t col = 0;
        bool lastWasNum = false;

        while (next != view.end() && row <= 7) {
            ASSERT(col <= 8);
            ASSERT(row <= 7);
            if (col == 8) {
                if (row == 0) {
                    if (*next == '/') {
                        return "Must not have trailing '/'";
                    }
                    return "Board is too long already data for _64_ squares";
                }
                if (*next != '/') {
                    return "Must have '/' as row separators";
                }

                ++next;
                --row;
                col = 0;
                lastWasNum = false;
                // just in case it was the final char
                continue;
            }
            if (std::isalpha(*next)) {
                auto piece = Piece::fromFEN(*next);
                if (!piece) {
                    return "Unknown piece type'" + std::string(1, *next) + "'";
                }
                setPiece(col, row, *piece);
                ++col;
                lastWasNum = false;
            } else {
                if (!std::isdigit(*next)) {
                    return "Invalid character '" + std::string(1, *next) + "'";
                }
                if (lastWasNum) {
                    return "Multiple consecutive numbers is not allowed";
                }
                uint32_t val = *next - '0';
                if (val == 0) {
                    return "Skipping 0 is not allowed";
                }
                if (val > size || val > (size - col)) {
                    return "Skipping more than a full row or the current row _" + std::to_string(val) + "_";
                }
                col += val;

                lastWasNum = true;
            }

            ++next;
        }


        if (row > 0 || col != 8) {
            return "Not enough data to fill board only reached row: "
                   + std::to_string(row) + " col: " + std::to_string(col);
        }

        // remember nullopt means no error here...
        return std::nullopt;
    }

    char turnColor(Color color) {
        if (color == Color::White) {
            return 'w';
        }
        return 'b';
    }

    std::optional<Color> parseTurnColor(const std::string_view& vw) {
        if (vw.size() != 1) {
            return std::nullopt;
        }

        if (vw[0] == 'w') {
            return Color::White;
        } else if (vw[0] == 'b') {
            return Color::Black;
        }

        return std::nullopt;
    }

    std::optional<uint32_t> strictParseUInt(const std::string_view& sv) {
        uint32_t result;
        if (sv.size() > 1 && sv[0] == '0') {
            // NO LEADING ZEROS!
            return std::nullopt;
        }
        if(auto [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
                ec == std::errc()) {
            if (p != sv.data() + sv.size()) {
                return std::nullopt;
            }
            return result;
        }
        return std::nullopt;
    }


    ExpectedBoard Board::fromFEN(std::string_view str) {
#ifdef OUTPUT_FEN
        std::cout << str << '\n';
#endif
        Board b{};

        //rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

        std::vector<std::string_view> parts = util::split(str, " ");
        if (parts.size() != 6) {
            return "Not enough pieces in FEN";
        }

        auto error = b.parseFENBoard(parts[0]);
        if (error) {
            return error.value();
        }

        std::optional<Color> nextTurn = parseTurnColor(parts[1]);
        if (!nextTurn.has_value()) {
            return std::string("Invalid turn value: ") + std::string(parts[1]);
        }
        b.m_nextTurnColor = nextTurn.value();

        auto castling = b.setAvailableCastles(parts[2]);
        if (castling.has_value()) {
            return std::string("Invalid possible castling moves value: ") + castling.value() + "(" + std::string(parts[2]) + ")";
        }

        if (parts[3] != "-") {
            std::optional<BoardIndex> enPassantPawn = Board::SANToIndex(parts[3]);
            if (!enPassantPawn.has_value()) {
                return std::string("Invalid en passant value: ") + std::string(parts[3]);
            }
            auto [col, row] = Board::indexToColumnRow(*enPassantPawn);
            Color lastMoveColor = opposite(b.m_nextTurnColor);
            if ((lastMoveColor == Color::White && row != 2) || (lastMoveColor == Color::Black && row != size - 1 - 2)) {
                return std::string("Cannot have en passant on non 3th or 5th row: " + std::string(parts[3]));
            }
            if (b.pieceAt(col, row) != std::nullopt) {
                return "En passant square cannot have a piece at square";
            }
            BoardIndex pawnRow = row + (lastMoveColor == Color::White ? 1 : -1);
            if (b.pieceAt(col, pawnRow) != Piece{Piece::Type::Pawn, lastMoveColor}) {
                return "En passant square must be just behind previously moved pawn";
            }
            b.m_enPassant = enPassantPawn;
        }

        std::optional<uint32_t> halfMovesSinceCapture = strictParseUInt(parts[4]);
        if (!halfMovesSinceCapture.has_value()) {
            // must draw after 75 full moves on not capturing but it is valid FEN....
            return std::string("Invalid half moves since capture: ") + std::string(parts[4]);
        }
        b.m_halfMovesSinceCaptureOrPawn = halfMovesSinceCapture.value();

        std::optional<uint32_t> totalFullMoves = strictParseUInt(parts[5]);
        if (!totalFullMoves.has_value() || totalFullMoves == 0u) {
            return std::string("Invalid full moves made: ") + std::string(parts[5]);
        }
        if (totalFullMoves >= (std::numeric_limits<uint32_t>::max() / 2u - 3u)) {
            return std::string("Too many full moves") + std::string(parts[5]);
        }
        b.m_halfMovesMade = (totalFullMoves.value() - 1) * 2 + (b.m_nextTurnColor == Color::Black);

        return b;
    }

    std::string Board::toFEN() const {
        std::stringstream val;

        BoardIndex emptyAcc = 0;

        auto writeEmpty = [&] {
          if (emptyAcc > 0) {
              val << std::to_string(emptyAcc);
              emptyAcc = 0;
          }
        };

        for (BoardIndex row = size - 1; row < size; row--) {
            for (BoardIndex column = 0; column < size; column++) {
                auto nextPiece = pieceAt(column, row);
                if (nextPiece) {
                    writeEmpty();
                    val << nextPiece->toFEN();
                } else {
                    emptyAcc++;
                }
            }
            writeEmpty();
            if (row != 0) {
                val << '/';
            }
        }

        val << ' ' << turnColor(m_nextTurnColor)
            << ' ' << m_castlingRights
            << ' ' << (m_enPassant.has_value() ? indexToSAN(m_enPassant.value()) : "-")
            << ' ' << m_halfMovesSinceCaptureOrPawn
            << ' ' << fullMoves();

        return val.str();
    }

}
