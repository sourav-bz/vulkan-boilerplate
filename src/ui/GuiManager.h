#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <functional>

class VulkanDevice;
class VulkanSwapchain;

class GuiManager {
public:
    struct Config {
        uint32_t maxFramesInFlight = 2;
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    };

    GuiManager(const Config& config);
    ~GuiManager();

    void initialize(GLFWwindow* window, VkInstance instance, VulkanDevice* device, 
                   VulkanSwapchain* swapchain, VkRenderPass renderPass);
    void cleanup();
    
    void newFrame();
    void render(VkCommandBuffer commandBuffer);
    void setupDocking();
    
    // UI callback - you can set this to define your UI
    std::function<void()> uiCallback;

private:
    Config config_;
    VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
    VulkanDevice* vulkanDevice_ = nullptr;
    
    void createDescriptorPool();
    void destroyDescriptorPool(); 
};