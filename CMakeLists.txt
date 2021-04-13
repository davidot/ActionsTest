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
        src/chess/Board.cpp
        src/chess/Fen.cpp
        src/chess/Move.cpp
        src/chess/MoveGen.cpp
        src/chess/Piece.cpp
        src/chess/SAN.cpp

        src/util/Assertions.cpp
        src/util/StringUtil.cpp
        )

target_include_directories(Actions INTERFACE src/)

add_executable(ActionsMain src/main.cpp)
target_link_libraries(ActionsMain PRIVATE Actions)

if (NOT HEADLESS)
    include(cmake/dependencies/GUI.cmake)
    add_subdirectory(visual)
endif()


option(EXTENDED_TESTS "Run full tests" OFF)
option(WITH_BENCHMARKS "Enable and run benchmarks" OFF)

include(cmake/dependencies/catch.cmake)
add_executable(ActionsTest test/main.cpp
        test/util/StringUtil.cpp
        test/chess/TestUtil.cpp
        test/chess/Piece.cpp test/chess/Board.cpp test/chess/MoveGen.cpp test/chess/Moves.cpp)
target_link_libraries(ActionsTest PRIVATE Catch2::Catch2 Actions)
if (EXTENDED_TESTS)
    target_compile_definitions(ActionsTest PRIVATE EXTENDED_TESTS=1)
    message(STATUS "Running full tests")
endif()
if (WITH_BENCHMARKS)
    target_compile_definitions(ActionsTest PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING=1)
    target_sources(ActionsTest PRIVATE test/chess/Benchmarks.cpp)
    message(STATUS "Building with benchmarks")
endif()


enable_testing()

add_test(NAME UnitTests COMMAND ActionsTest)

add_test(NAME StartUpTest COMMAND ActionsMain)

add_subdirectory(fuzzer)
