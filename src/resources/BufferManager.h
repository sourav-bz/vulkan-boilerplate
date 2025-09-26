#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../core/VulkanDevice.h"
#include "../rendering/CommandManager.h"
#include "../common/Vertex.h"
#include "../common/VertexTypes.h"

struct UniformBufferObject;

class BufferManager {
    public:
        BufferManager();
        ~BufferManager();

        void initialize(const VulkanDevice& device, CommandManager& commandManager);
        void cleanup();

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void createVertexBuffer(const std::vector<StandardVertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
        void createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
        void createUniformBuffer(uint32_t maxFramesInFlight, std::vector<VkBuffer>& uniformBuffers, 
                             std::vector<VkDeviceMemory>& uniformBuffersMemory, 
                             std::vector<void*>& uniformBuffersMapped);

        void destroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    private:
        const VulkanDevice* vulkanDevice = nullptr;
        CommandManager* commandManager = nullptr;
};