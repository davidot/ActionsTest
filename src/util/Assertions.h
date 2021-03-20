#pragma once

namespace util::Assert {
    void assertFailed(const char *assertion, const char *file, int line);
}

#ifndef NDEBUG
#define ASSERT(expr)                                         \
    do {                                                     \
        if (!static_cast<bool>(expr)) {                      \
            util::Assert::assertFailed(#expr, __FILE__, __LINE__); \
        }                                                    \
    } while (0)

#else
#define ASSERT(expr)
#endif

#define ASSERT_NOT_REACHED() ASSERT(false)