cmake_minimum_required(VERSION 3.16)
project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(SOURCES
    src/main.cpp
    src/core/VulkanInstance.cpp
    src/core/VulkanDevice.cpp
)

add_executable(vulkan_triangle ${SOURCES})
target_link_libraries(vulkan_triangle ${Vulkan_LIBRARIES} glfw glm::glm)
target_include_directories(vulkan_triangle PRIVATE ${Vulkan_INCLUDE_DIRS} external/stb external/tiny_obj)