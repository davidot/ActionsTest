
add_executable(BatchFEN BatchFENTest.cpp)
target_link_libraries(BatchFEN PRIVATE Actions)

add_executable(ProcTestApp EXCLUDE_FROM_ALL ProcTestProc.cpp)
add_executable(ProcTest ProcTest.cpp)
target_link_libraries(ProcTest PRIVATE Actions)
add_dependencies(ProcTest ProcTestApp)

add_test(NAME ProcTestTest COMMAND ProcTest $<TARGET_FILE:ProcTestApp>)
set_tests_properties(ProcTestTest PROPERTIES TIMEOUT 60)
if(WIN32)
    set_tests_properties(ProcTestTest PROPERTIES DISABLED True)
endif()
