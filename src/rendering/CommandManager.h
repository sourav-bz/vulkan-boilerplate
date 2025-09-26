#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../core/VulkanDevice.h"

class CommandManager {
    public:
        CommandManager();
        ~CommandManager();

        void initialize(const VulkanDevice& device, uint32_t maxFramesInFlight);
        void cleanup();

        VkCommandPool getCommandPool() const { return commandPool; }
        const std::vector<VkCommandBuffer>& getCommandBuffers() const { return commandBuffers; }
        VkCommandBuffer getCommandBuffer(uint32_t frameIndex) const { return commandBuffers[frameIndex]; }

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commmandBuffer);

        void resetCommandBuffer(uint32_t frameIndex);
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, 
                           VkRenderPass renderPass, VkFramebuffer framebuffer,
                           VkExtent2D extent, VkPipeline graphicsPipeline,
                           VkPipelineLayout pipelineLayout, VkBuffer vertexBuffer,
                           VkBuffer indexBuffer, const std::vector<VkDescriptorSet>& descriptorSets,
                           uint32_t currentFrame, uint32_t indexCount);

    private:
        const VulkanDevice* vulkanDevice = nullptr;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers;

        void createCommandPool();
        void createCommandBuffers(uint32_t maxFramesInFlight);
};