
add_executable(BatchFEN BatchFENTest.cpp)
target_link_libraries(BatchFEN PRIVATE Actions)

add_executable(ProcTestApp ProcTestProc.cpp)
add_executable(ProcTest ProcTest.cpp)
target_link_libraries(ProcTest PRIVATE Actions)

set(TestAppLocation "$<TARGET_FILE:ProcTestApp>")

message(STATUS "ProcTestApp @ ${TestAppLocation}")

add_test(NAME ProcTestTest COMMAND ProcTest "${TestAppLocation}")
