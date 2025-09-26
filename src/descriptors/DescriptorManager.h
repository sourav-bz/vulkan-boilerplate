#pragma once 

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include "../core/VulkanDevice.h"

struct UniformBufferObject;

class DescriptorManager {
    public:
        DescriptorManager();
        ~DescriptorManager();

        void initialize(const VulkanDevice& device);
        void cleanup();

        void createDescriptorPool(uint32_t maxFramesInFlight);
        void createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, 
                             uint32_t maxFramesInFlight,
                             const std::vector<VkBuffer>& uniformBuffers,
                             VkImageView textureImageView,
                             VkSampler textureSampler,
                             std::vector<VkDescriptorSet>& descriptorSets);

        VkDescriptorPool getDescriptorPool() const { return descriptorPool; }

        void destroyDescriptorPool();

    private:
        const VulkanDevice* vulkanDevice = nullptr;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
};