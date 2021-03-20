#pragma once

namespace util::Assert {
    void assertFailed(const char *assertion, const char *file, int line, const char *func);
}

#ifndef NDEBUG
#define ASSERT(expr)                                         \
    do {                                                     \
        if (!static_cast<bool>(expr)) {                      \
            util::Assert::assertFailed(#expr, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        }                                                    \
    } while (0)

#else
#define ASSERT(expr)
#endif

#define ASSERT_NOT_REACHED() ASSERT(false)