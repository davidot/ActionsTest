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


fetchcontent_declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG f1bf642e8f9b2a2ecbe03de10743bd1e504cb3f8
        GIT_PROGRESS TRUE
)


FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    #    add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

fetchcontent_declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml
        GIT_TAG 4b3ce009be611570d76480895f492c9734bf62ff
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(imgui-sfml)
if (NOT imgui-sfml_POPULATED)
    FetchContent_Populate(imgui-sfml)

    # Set relevant vars
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
    set(IMGUI_SFML_FIND_SFML CACHE BOOL FALSE)
    set(IMGUI_SFML_IMGUI_DEMO ON)

    message(STATUS ${imgui-sfml_SOURCE_DIR} , ${imgui-sfml_BINARY_DIR})

    add_subdirectory(${imgui-sfml_SOURCE_DIR} ${imgui-sfml_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()


fetchcontent_declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot
        GIT_TAG 46ea9abab4bb8b5fb60ce6b61f6e5c1bf2e840c1
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(implot)
if (NOT implot_POPULATED)
    FetchContent_Populate(implot)

    add_library(Implot INTERFACE)
    target_sources(Implot INTERFACE ${implot_SOURCE_DIR}/implot.h
                                    ${implot_SOURCE_DIR}/implot_internal.h
                                    ${implot_SOURCE_DIR}/implot.cpp
                                    ${implot_SOURCE_DIR}/implot_items.cpp
                                    ${implot_SOURCE_DIR}/implot_demo.cpp)
    target_include_directories(Implot INTERFACE ${implot_SOURCE_DIR})
endif ()


