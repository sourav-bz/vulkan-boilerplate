#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "VulkanSwapchain.h"
#include "../core/VulkanDevice.h"

class VulkanGraphicsPipeline{
    public:
        VulkanGraphicsPipeline();
        ~VulkanGraphicsPipeline();

        void initialize(const VulkanDevice& device, const VulkanSwapchain& swapchain);
        void cleanup();
        void recreate(const VulkanSwapchain& swapchain);

        VkRenderPass getRenderPass() const { return renderPass; }
        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
        VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
        VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }

        bool hasStencilComponent(VkFormat format);
        VkFormat findDepthFormat();

    private:
        const VulkanDevice* vulkanDevice = nullptr;
        const VulkanSwapchain* vulkanSwapchain = nullptr;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;

        void createRenderPass(VkFormat swapChainImageFormat);
        void createDescriptorSetLayout();
        void createGraphicsPipeline(VkExtent2D swapChainExtent);

        VkShaderModule createShaderModule(const std::vector<char>& code);
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        

        std::vector<char> readFile(const std::string& filename);

};