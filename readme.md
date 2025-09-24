## Vulkan Boilerplate Setup

### Tech stack & Libraries:

- Vulkan
- C++
- GLFW
- stb_image
- ImGUI

### Folder structure

```
├── CMakeLists.txt
├── readme.md
├── .gitignore
├── src/
│ ├── main.cpp
│ │
│ ├── core/
│ │ ├── Application.cpp
│ │ ├── VulkanInstance.cpp
│ │ ├── VulkanDevice.cpp
│ │ └── VulkanValidation.h
│ │
│ ├── rendering/
│ │ ├── VulkanRenderer.cpp
│ │ ├── Swapchain.cpp
│ │ ├── RenderPass.cpp
│ │ ├── GraphicsPipeline.cpp
│ │ └── CommandManager.h
│ │
│ ├── resources/
│ │ ├── Buffer.cpp
│ │ ├── Image.cpp
│ │ ├── Texture.cpp
│ │ ├── Mesh.cpp
│ │ └── Shader.h
│ │
│ ├── descriptors/
│ │ ├── DescriptorSetLayout.cpp
│ │ ├── DescriptorPool.cpp
│ │ └── DescriptorSet.cpp
│ │
│ ├── scene/
│ │ ├── Camera.cpp
│ │ ├── Transform.h
│ │ ├── Model.cpp
│ │ └── Scene.h
│ │
│ ├── input/
│ │ └── InputManager.cpp
│ │
│ └── utils/
│ ├── FileUtils.cpp
│ ├── VulkanUtils.cpp
│ └── Math.h
```
