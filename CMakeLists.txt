cmake_minimum_required(VERSION 3.16)
project(VulkanBoilerplate)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(external/imgui-cmake)

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
    src/ui/GuiManager.cpp
)

add_executable(vulkan_boilerplate ${SOURCES})
target_link_libraries(vulkan_boilerplate ${Vulkan_LIBRARIES} glfw glm::glm imgui)
target_include_directories(vulkan_boilerplate PRIVATE 
    ${Vulkan_INCLUDE_DIRS} 
    external/stb 
    external/tiny_obj 
    external/imgui
    external/imgui/backends
)