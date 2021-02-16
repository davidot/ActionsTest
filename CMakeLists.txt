cmake_minimum_required(VERSION 3.17)
project(ActionsTest LANGUAGES CXX
        VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

fetchcontent_declare(
        catch
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.1
)

FetchContent_GetProperties(catch)
if (NOT catch_POPULATED)
    FetchContent_Populate(catch)
    add_subdirectory(${catch_SOURCE_DIR} ${catch_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()


add_library(Actions src/Action.cpp src/chess/Piece.cpp src/chess/Board.cpp)
target_include_directories(Actions INTERFACE src/)

add_executable(ActionsMain src/main.cpp)
target_link_libraries(ActionsMain PRIVATE Actions)


add_executable(ActionsTest test/main.cpp test/ActionTest.cpp test/chess/Piece.cpp test/chess/Board.cpp)
target_link_libraries(ActionsTest PRIVATE Catch2::Catch2 Actions)

enable_testing()

add_test(NAME UnitTests COMMAND ActionsTest)

add_test(NAME StartUpTest COMMAND ActionsMain)
