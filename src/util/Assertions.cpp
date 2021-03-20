#include "Assertions.h"
#include <iostream>

namespace util::Assert {
    void assertFailed(const char *assertion, const char *file, int line) {
        std::cerr << "Assertion: " << assertion << " failed! " << file << ':' << line << " (in " << func << ")\n";
#if defined __has_builtin
#  if __has_builtin (__builtin_trap)
        __builtin_trap();
#  endif
#endif
        std::exit(-3);
    }
}
