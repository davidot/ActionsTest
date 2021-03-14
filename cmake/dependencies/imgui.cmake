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


