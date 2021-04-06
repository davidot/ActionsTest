#include <chess/Board.h>
#include <chess/MoveGen.h>
#include <iostream>


extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    using namespace Chess;
    std::string value((char*)Data, Size);
    ExpectedBoard parsed = Board::fromFEN(value);
    if (parsed) {
        // write FEN should be indentical

        std::string output = parsed.value().toFEN();
        if (value != output) {
            std::cout << "Not equal in and output FEN\n"
                      << " Input: " << value << '\n'
                      << "Output: " << output << '\n';
            __builtin_trap();
        }

        Board copy = parsed.value();
        Board board = parsed.extract();
        if (!(copy == board)) {
            __builtin_trap();
        }

        auto list = Chess::generateAllMoves(board);

        list.forEachMove([&](const Move& move) {
            if (board.undoMove()) {
                std::cout << "Could undo move while there was none!\n";
                __builtin_trap();
            }
            if (!board.makeMove(move)) {
                std::cout << "Generated illegal move\n";
                __builtin_trap();
            }
            if (!board.undoMove()) {
                std::cout << "Could not perform undo\n";
                __builtin_trap();
            }
            if (!(copy == board)) {
                std::cout << "Board not the same after undo\n";
                __builtin_trap();
            }
        });

    } else {
        if (parsed.error().empty()) {
            std::cout << "No error specified!\n";
            __builtin_trap();
        }
    }
    return 0;
}