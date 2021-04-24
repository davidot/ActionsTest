fetchcontent_declare(
        catch
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG 2c269eb6332bc1dd29047851ae1efc3cd4c260d2
)

FetchContent_GetProperties(catch)
if (NOT catch_POPULATED)
    FetchContent_Populate(catch)
    add_subdirectory(${catch_SOURCE_DIR} ${catch_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
