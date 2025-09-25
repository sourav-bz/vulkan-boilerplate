#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>
#include "../core/VulkanDevice.h"

class VulkanSwapchain{
    public:
        VulkanSwapchain();
        ~VulkanSwapchain();

        void initialize(const VulkanDevice& device, VkSurfaceKHR surface, GLFWwindow* window);
        void cleanup();
        void recreate(GLFWwindow* window);

        VkSwapchainKHR getSwapChain() const { return swapChain; }
        const std::vector<VkImage>& getImages() const { return swapChainImages; }
        VkFormat getImageFormat() const { return swapChainImageFormat; }
        VkExtent2D getExtent() const { return swapChainExtent; }
        const std::vector<VkImageView>& getImageViews() const { return swapChainImageViews; }

    private:
        const VulkanDevice* vulkanDevice = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;

        void createSwapChain(GLFWwindow* window);
        void createImageViews();
        void cleanupSwapChain();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
};