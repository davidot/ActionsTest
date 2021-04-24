#include <catch2/catch_test_macros.hpp>
#include <util/StringUtil.h>

TEST_CASE("Splitting strings", "[util]") {

    using namespace util;

    SECTION("Separator does not occur") {
        std::string str = "This is a test";
        std::string separator = "x";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == str);
    }

    SECTION("Separator does not occur because it is longer") {
        std::string str = "This is a test";
        std::string separator = "x";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == str);
    }

    SECTION("Simple example") {
        std::string str = "This is a test";
        std::string separator = " ";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 4);
        CHECK(parts[0] == "This");
        CHECK(parts[1] == "is");
        CHECK(parts[2] == "a");
        CHECK(parts[3] == "test");
    }

    SECTION("Has separator at end") {
        std::string str = "test ";
        std::string separator = " ";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 2);
        CHECK(parts[0] == "test");
        CHECK(parts[1] == "");
    }

    SECTION("Looks at full separator") {
        std::string str = "this is a  test";
        std::string separator = "  ";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 2);
        CHECK(parts[0] == "this is a");
        CHECK(parts[1] == "test");
    }

    SECTION("Looks at full separator at end") {
        std::string str = "this is a test  ";
        std::string separator = "  ";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 2);
        CHECK(parts[0] == "this is a test");
        CHECK(parts[1] == "");
    }

    SECTION("Looks at full separator at beginning") {
        std::string str = "  this is a test";
        std::string separator = "  ";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 2);
        CHECK(parts[0] == "");
        CHECK(parts[1] == "this is a test");
    }

    SECTION("Empty string gives single part") {
        std::string str = "";
        std::string separator = " ";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "");
    }

    SECTION("Empty string gives single part even with empty separator") {
        std::string str = "";
        std::string separator = "";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "");
    }

    SECTION("Empty string gives single part even with empty separator") {
        std::string str = "abcde";
        std::string separator = "";
        auto parts = split(str, separator);
        REQUIRE(parts.size() == 5);
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "b");
        CHECK(parts[2] == "c");
        CHECK(parts[3] == "d");
        CHECK(parts[4] == "e");
    }

}
