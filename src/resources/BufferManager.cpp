#include "BufferManager.h"

#include <iostream>
#include <stdexcept>
#include <cstring>

struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

BufferManager::BufferManager() {}

BufferManager::~BufferManager(){
    cleanup();
}

void BufferManager::initialize(const VulkanDevice& device, CommandManager& cmdManager){
    vulkanDevice = &device;
    commandManager = &cmdManager;
}

void BufferManager::cleanup(){

}

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory){
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanDevice->getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanDevice->getLogicalDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkanDevice->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkanDevice->getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(vulkanDevice->getLogicalDevice(), buffer, bufferMemory, 0);
}

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){
    VkCommandBuffer commandBuffer = commandManager->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    commandManager->endSingleTimeCommands(commandBuffer);
}


void BufferManager::createUniformBuffer(uint32_t maxFramesInFlight, std::vector<VkBuffer>& uniformBuffers, 
                             std::vector<VkDeviceMemory>& uniformBuffersMemory, 
                             std::vector<void*>& uniformBuffersMapped){
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(maxFramesInFlight);
    uniformBuffersMemory.resize(maxFramesInFlight);
    uniformBuffersMapped.resize(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(vulkanDevice->getLogicalDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void BufferManager::createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory){
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(vulkanDevice->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(vulkanDevice->getLogicalDevice(), stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(vulkanDevice->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanDevice->getLogicalDevice(), stagingBufferMemory, nullptr);
}

void BufferManager::createVertexBuffer(const std::vector<StandardVertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory){
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(vulkanDevice->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(vulkanDevice->getLogicalDevice(), stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(vulkanDevice->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanDevice->getLogicalDevice(), stagingBufferMemory, nullptr);
}


void BufferManager::destroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vulkanDevice->getLogicalDevice(), buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }
    if (bufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vulkanDevice->getLogicalDevice(), bufferMemory, nullptr);
        bufferMemory = VK_NULL_HANDLE;
    }
}