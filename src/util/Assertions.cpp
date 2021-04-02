#include "Assertions.h"
#include <iostream>

namespace util::Assert {
    void assertExpression(bool passed, const char *assertion, const char *file, int line) {
        if (!passed) {
            std::cerr << "Assertion: " << assertion << " failed! in " << file << ':' << line << "\n";
#if defined __has_builtin
#if __has_builtin(__builtin_trap)
            __builtin_trap();
#endif
#endif
            std::exit(-3);
        }
    }
}
