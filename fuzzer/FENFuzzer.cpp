#include <chess/Board.h>
#include <iostream>


extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    using namespace Chess;
    std::string value((char*)Data, Size);
    ExpectedBoard parsed = Board::fromFEN(value);
    if (parsed) {
        // write FEN should be indentical
        if (value != parsed.value().toFEN()) {
            std::cout << "Not equal in and output FEN\n";
            __builtin_trap();
        }
    } else {
        if (parsed.error().empty()) {
            std::cout << "No error specified!\n";
            __builtin_trap();
        }
    }
    return 0;
}