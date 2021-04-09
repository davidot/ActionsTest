#pragma once

namespace util::Assert {
    void assertExpression(bool passed, const char *assertion, const char *file, int line);

    [[noreturn]] void unreachable(const char* file, int line);
}// namespace util::Assert

#ifndef NDEBUG
#define ASSERT(expr)                                                                        \
    do {                                                                                    \
        util::Assert::assertExpression(static_cast<bool>(expr), #expr, __FILE__, __LINE__); \
    } while (0)

#define ASSERT_NOT_REACHED()          \
    do {                              \
        util::Assert::unreachable(__FILE__, __LINE__); \
    } while (0)
#else
#define ASSERT(expr)

#define ASSERT_NOT_REACHED()
#endif
