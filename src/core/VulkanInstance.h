#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanInstance{
    public:
        VulkanInstance();
        ~VulkanInstance();

        VulkanInstance(const VulkanInstance&) = delete;
        VulkanInstance& operator=(const VulkanInstance&) = delete;

        void initialize();
        void cleanup();

        VkInstance getInstance() const { return instance; }
        bool isValidationEnabled() const { return enableValidationLayers; }
        const std::vector<const char*>& getValidationLayers() const { return validationLayers; }

    private:
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        bool enableValidationLayers;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        void createInstance();
        void setupDebugMessenger();
        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
        
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator);