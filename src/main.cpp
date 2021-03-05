#include <chess/Board.h>
#include <iostream>

int main() {
    Chess::Board board = Chess::Board::standardBoard();
    std::cout << "Standard fen: " << board.toFEN() << '\n';
    return 0;
}
