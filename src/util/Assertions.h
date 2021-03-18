#pragma once

#ifndef NDEBUG
#include <iostream>
#define ASSERT(expr)                                                                          \
    do {                                                                                      \
        if (!static_cast<bool>(expr)) {                                                       \
            std::cout << "VERIFY: " << #expr << " failed in " << __FILE__ << ":" << __LINE__; \
            std::exit(-3);                                                                    \
        }                                                                                     \
    } while (0)

#else
#define VERIFY(expr)
#endif

#define VERIFY_NOT_REACHED() ASSERT(false)