fetchcontent_declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML
        GIT_TAG 2.5.1
        GIT_PROGRESS TRUE
)

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

if(WIN32)
    set(SFML_USE_STATIC_STD_LIBS TRUE CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)
endif()

FetchContent_GetProperties(sfml)
if (NOT sfml_POPULATED)
    FetchContent_Populate(sfml)
    add_subdirectory(${sfml_SOURCE_DIR} ${sfml_BINARY_DIR} EXCLUDE_FROM_ALL)
    set_property(TARGET sfml-system PROPERTY INTERPROCEDURAL_OPTIMIZATION False)
    set_property(TARGET sfml-window PROPERTY INTERPROCEDURAL_OPTIMIZATION False)
    set_property(TARGET sfml-graphics PROPERTY INTERPROCEDURAL_OPTIMIZATION False)
    if (TARGET sfml-main)
        set_property(TARGET sfml-main PROPERTY INTERPROCEDURAL_OPTIMIZATION False)
    endif ()
endif ()
