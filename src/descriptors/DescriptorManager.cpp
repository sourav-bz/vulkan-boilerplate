#include "DescriptorManager.h"
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>

struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

DescriptorManager::DescriptorManager() {}

DescriptorManager::~DescriptorManager() {
    cleanup();
}

void DescriptorManager::initialize(const VulkanDevice& device) {
    vulkanDevice = &device;
}

void DescriptorManager::cleanup() {
    destroyDescriptorPool();
}

void DescriptorManager::createDescriptorPool(uint32_t maxFramesInFlight){
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);

    VkResult descriptorPoolResult = vkCreateDescriptorPool(vulkanDevice->getLogicalDevice(), &poolInfo, nullptr, &descriptorPool);
    if (descriptorPoolResult != VK_SUCCESS) {
        std::cout << "failed to create descriptor pool! - " << descriptorPoolResult << std::endl;
        throw std::runtime_error("failed to create descriptor pool!");
    }else{
        std::cout << "Successfully created descriptor pool - " << descriptorPoolResult << std::endl;
    }
}

void DescriptorManager::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, 
                             uint32_t maxFramesInFlight,
                             const std::vector<VkBuffer>& uniformBuffers,
                             VkImageView textureImageView,
                             VkSampler textureSampler,
                             std::vector<VkDescriptorSet>& descriptorSets){
    std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(maxFramesInFlight);
    VkResult descriptorSetsResult = vkAllocateDescriptorSets(vulkanDevice->getLogicalDevice(), &allocInfo, descriptorSets.data());
    if (descriptorSetsResult != VK_SUCCESS) {
        std::cout << "failed to allocate descriptor sets! - " << descriptorSetsResult << std::endl;
        throw std::runtime_error("failed to allocate descriptor sets!");
    }else{
        std::cout << "Successfully created descriptor sets - " << descriptorSetsResult << std::endl;
    }

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(vulkanDevice->getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void DescriptorManager::destroyDescriptorPool() {
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(vulkanDevice->getLogicalDevice(), descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
}