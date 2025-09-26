cmake_minimum_required(VERSION 3.16)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(imgui PUBLIC ${Vulkan_LIBRARIES} glfw)
target_include_directories(imgui PUBLIC ${Vulkan_INCLUDE_DIRS})