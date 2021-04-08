#include <chess/Board.h>
#include <chess/MoveGen.h>
#include <iostream>

int main() {
    Chess::Board board = Chess::Board::standardBoard();

    std::cout << "Start fen: " << board.toFEN() << '\n';

    Chess::MoveList list = Chess::generateAllMoves(board);

    std::cout << "Found " << list.size() << " moves from start position\n";

    return 0;
}
