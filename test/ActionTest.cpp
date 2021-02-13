#include <catch2/catch.hpp>
#include <Action.h>

TEST_CASE("BasicActionTest") {
    int v = GENERATE(-3, 3, 10, 0, 11);
    Action a{v};
    REQUIRE(a.getA() == v);
}
