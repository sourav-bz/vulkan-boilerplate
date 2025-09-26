# Vulkan Boilerplate

A Vulkan rendering engine boilerplate written in C++.

## Dependencies

- Vulkan SDK
- GLFW3
- GLM
- ImGui
- stb_image
- tiny_obj_loader

## Build Instructions

Clone the repository with submodules:

```bash
git clone --recursive <repository-url>
```

Or if you've already cloned it, initialize the submodules:

```bash
git submodule update --init --recursive
```

Then build the project:

```bash
mkdir build
cd build
cmake ..
make
```

## Project Structure

```
src/
├── main.cpp
├── common/           # Vertex definitions and types
├── core/            # Vulkan instance, device, and application
├── rendering/       # Swapchain, graphics pipeline, commands
├── resources/       # Buffer and texture management
├── descriptors/     # Descriptor set management
└── ui/              # ImGui integration

assets/
├── fonts/           # Font files
├── models/          # 3D models
└── textures/        # Texture files

shaders/             # GLSL shaders and compiled SPIR-V

external/
├── imgui/           # Dear ImGui submodule
├── imgui-cmake/     # CMake wrapper for ImGui
├── stb/             # STB image loading library
└── tiny_obj/        # Tiny OBJ loader
```

## Submodules

This project uses Dear ImGui as a git submodule. When cloning, use the `--recursive` flag to automatically get all submodules.
