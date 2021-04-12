#include "Assertions.h"
#include <iostream>

namespace util::Assert {

    [[noreturn]] void assertFailed() {
#ifdef _MSC_VER
        __debugbreak();
#else
        __builtin_trap();
#endif
        std::exit(-3);
    }


    void assertExpression(bool passed, const char *assertion, const char *file, int line) {
        if (!passed) {
            std::cerr << "Assertion: " << assertion << " failed! in " << file << ':' << line << std::endl;

            assertFailed();
        }
    }

    void unreachable(const char* file, int line) {
        std::cerr << "Hit unreachable spot in " << file << ":" << line << std::endl;

        assertFailed();
    }

}
