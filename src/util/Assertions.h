#pragma once

namespace util::Assert {
    void assertExpression(bool passed, const char *assertion, const char *file, int line);
}

#ifndef NDEBUG
#define ASSERT(expr)                                         \
    do {                                                     \
        util::Assert::assert(static_cast<bool>(expr), #expr, __FILE__, __LINE__);                                                     \
    } while (0)

#else
#define ASSERT(expr)
#endif

#define ASSERT_NOT_REACHED() ASSERT(false)