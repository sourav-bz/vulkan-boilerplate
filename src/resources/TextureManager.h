#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "../core/VulkanDevice.h"
#include "../rendering/CommandManager.h"
#include "../resources/BufferManager.h"

class TextureManager{
    public:
        TextureManager();
        ~TextureManager();

        void initialize(const VulkanDevice& device, CommandManager& commandManager, BufferManager& bufferManager);
        void cleanup();

        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
                    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
                    VkImage& image, VkDeviceMemory& imageMemory);

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void createTextureFromFile(const std::string& texturePath, VkImage& textureImage, 
                              VkDeviceMemory& textureImageMemory, VkImageView& textureImageView);
        VkSampler createTextureSampler();

        void createDepthResources(VkExtent2D extent, VkImage& depthImage, 
                             VkDeviceMemory& depthImageMemory, VkImageView& depthImageView);

        void destroyImage(VkImage& image, VkDeviceMemory& imageMemory);
        void destroyImageView(VkImageView& imageView);
        void destroySampler(VkSampler& sampler);

    private:
        const VulkanDevice* vulkanDevice = nullptr;
        CommandManager* commandManager = nullptr;
        BufferManager* bufferManager = nullptr;

        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        bool hasStencilComponent(VkFormat format);
};