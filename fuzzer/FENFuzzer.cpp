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
        [[maybe_unused]] auto list = Chess::generateAllMoves(parsed.value());
    } else {
        if (parsed.error().empty()) {
            std::cout << "No error specified!\n";
            __builtin_trap();
        }
    }
    return 0;
}