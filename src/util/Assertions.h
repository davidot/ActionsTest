#pragma once

#include <iostream>
#define BASE_VERIFY(expr) \
    do {                  \
        /*GCOVR_EXCL_START*/ \
        if (!static_cast<bool>(expr)) { \
            std::cout << "VERIFY: " << #expr << " failed in " << __FILE__ << ":" << __LINE__; \
            std::exit(-3); \
        } \
    } while (0)           \
    /*GCOVR_EXCL_STOP*/

#define VERIFY(expr) BASE_VERIFY(expr)

#define VERIFY_NOT_REACHED() VERIFY(false)