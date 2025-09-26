#include <cstdint>
#include <string>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory> 

class VulkanInstance;
class VulkanDevice;
class VulkanSwapchain;
class VulkanGraphicsPipeline;
class CommandManager;
class BufferManager;
class TextureManager;
class DescriptorManager;

class VulkanApplication{
    public:
        struct Config {
            uint32_t windowWidth = 800;
            uint32_t windowHeight = 600;
            std::string windowTitle = "Vulkan Boilerplate";
            uint32_t maxFramesInFlight = 2;
            bool enableValidation = true;
        };

        VulkanApplication(const Config& config);
        virtual ~VulkanApplication();

        void run();

    protected:
        virtual void initializeResources() {}
        virtual void updateUniforms(uint32_t currentImage) {}
        virtual void recordRenderCommands(VkCommandBuffer commnadBuffer, uint32_t imageIndex) {}
        virtual void onCleanup() {} 

        Config config_;
        GLFWwindow* window_;
        VkSurfaceKHR surface_;

        std::unique_ptr<VulkanInstance> vulkanInstance_;
        std::unique_ptr<VulkanDevice> vulkanDevice_;
        std::unique_ptr<VulkanSwapchain> vulkanSwapchain_;
        std::unique_ptr<VulkanGraphicsPipeline> vulkanPipeline_;
        std::unique_ptr<CommandManager> commandManager_;
        std::unique_ptr<BufferManager> bufferManager_;
        std::unique_ptr<TextureManager> textureManager_;
        std::unique_ptr<DescriptorManager> descriptorManager_;

        std::vector<VkSemaphore> imageAvailableSemaphores_;
        std::vector<VkSemaphore> renderFinishedSemaphores_;
        std::vector<VkFence> inFlightFences_;
        uint32_t currentFrame_ = 0;
        bool framebufferResized_ = false;

    private:
        void initWindow();
        void initVulkan();
        void mainLoop();
        void drawFrame();
        void cleanup();
        void recreateSwapChain();
        void createSyncObjects();

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
