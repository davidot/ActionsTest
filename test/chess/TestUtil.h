#pragma once

#ifndef EXTENDED_TESTS
#define TEST_SOME(x) take(2, x)
#else
#define TEST_SOME(x) x
#endif