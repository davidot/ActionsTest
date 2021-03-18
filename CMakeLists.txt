cmake_minimum_required(VERSION 3.17)
project(ActionsTest LANGUAGES CXX
        VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

if (DEFINED ENV{CI})
    message(STATUS "Probably running in CI, skipping graphical things...")
    set(HEADLESS ON)
else ()
    set(HEADLESS OFF)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    message(STATUS "CCache loaded and configured")
endif(CCACHE_FOUND)
include(FetchContent)

add_library(Actions
        src/util/StringUtil.cpp
        src/chess/Piece.cpp src/chess/Board.cpp src/chess/MoveGen.cpp)

target_include_directories(Actions INTERFACE src/)

add_executable(ActionsMain src/main.cpp)
target_link_libraries(ActionsMain PRIVATE Actions)

if (NOT HEADLESS)
    include(cmake/dependencies/GUI.cmake)
    add_subdirectory(visual)
endif()


option(EXTENDED_TESTS "Run full tests" OFF)

include(cmake/dependencies/catch.cmake)
add_executable(ActionsTest test/main.cpp
        test/util/StringUtil.cpp
        test/chess/Piece.cpp test/chess/Board.cpp test/chess/MoveGen.cpp)
target_link_libraries(ActionsTest PRIVATE Catch2::Catch2 Actions)
if (EXTENDED_TESTS)
    target_compile_definitions(ActionsTest PRIVATE EXTENDED_TESTS=1)
endif()

enable_testing()

add_test(NAME UnitTests COMMAND ActionsTest)

add_test(NAME StartUpTest COMMAND ActionsMain)

add_subdirectory(fuzzer)
