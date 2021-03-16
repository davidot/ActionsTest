if (NOT CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" AND NOT APPLE)
    # TODO do not just exlucde apple but add cmake level option to build fuzzers
    message(STATUS "Fuzzers are only supported on Clang")
    return()
endif ()


function(add_simple_fuzzer fuzzer_source)
    add_executable(${fuzzer_source} ${fuzzer_source}.cpp)

    target_link_libraries(${fuzzer_source} PRIVATE
            Actions
            -coverage
            -fsanitize=fuzzer,undefined,address)

    target_compile_options(${fuzzer_source} PRIVATE -fsanitize=fuzzer,undefined,address)

# TODO: figure out how to do a dry run (probably in fuzzer workflow)
#    add_test(NAME ${fuzzer_source}_dry_test COMMAND ${fuzzer_source} -runs=0 ${fuzzer_source}_corpus)

    message(STATUS "Added fuzzer ${fuzzer_source}")
endfunction()

add_simple_fuzzer(FENFuzzer)