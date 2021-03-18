#include "Assertions.h"
#include <iostream>

namespace util::Assert {
    void assertFailed(const char *assertion, const char *file, int line, const char *func) {
        std::cerr << "Assertion: " << assertion << " failed! " << file << ':' << line << " (in " << func << ")\n";
        std::exit(-3);
    }
}
