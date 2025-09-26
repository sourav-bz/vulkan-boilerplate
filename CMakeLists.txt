cmake_minimum_required(VERSION 3.16)
project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(SOURCES
    src/main.cpp
    src/common/Vertex.cpp
    src/core/VulkanInstance.cpp
    src/core/VulkanDevice.cpp
    src/core/VulkanApplication.cpp
    src/rendering/VulkanSwapchain.cpp
    src/rendering/VulkanGraphicsPipeline.cpp
    src/rendering/CommandManager.cpp
    src/resources/BufferManager.cpp
    src/resources/TextureManager.cpp
    src/descriptors/DescriptorManager.cpp
)

add_executable(vulkan_triangle ${SOURCES})
target_link_libraries(vulkan_triangle ${Vulkan_LIBRARIES} glfw glm::glm)
target_include_directories(vulkan_triangle PRIVATE ${Vulkan_INCLUDE_DIRS} external/stb external/tiny_obj)