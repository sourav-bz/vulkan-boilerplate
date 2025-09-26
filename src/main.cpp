#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <array>
#include <unordered_map>

#include "common/Vertex.h"
#include "core/VulkanInstance.h"
#include "core/VulkanDevice.h"
#include "rendering/VulkanSwapchain.h"
#include "rendering/VulkanGraphicsPipeline.h"
#include "rendering/CommandManager.h"
#include "resources/BufferManager.h"
#include "resources/TextureManager.h"
#include "descriptors/DescriptorManager.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;
const std::string MODEL_PATH = "../assets/models/viking_room.obj";
const std::string TEXTURE_PATH = "../assets/textures/viking_room.png";
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    VulkanInstance vulkanInstance;
    VulkanDevice vulkanDevice;
    VulkanSwapchain vulkanSwapchain;
    VulkanGraphicsPipeline vulkanPipeline;
    CommandManager commandManager;
    BufferManager bufferManager;
    TextureManager textureManager;
    DescriptorManager descriptorManager;

    GLFWwindow* window;
    VkSurfaceKHR surface;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<VkDescriptorSet> descriptorSets;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    void updateUniformBuffer(uint32_t currentImage){
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkanSwapchain.getExtent().width / (float) vulkanSwapchain.getExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        vulkanInstance.initialize();
        createSurface();
        vulkanDevice.initialize(vulkanInstance, surface);
        vulkanSwapchain.initialize(vulkanDevice, surface, window);
        vulkanPipeline.initialize(vulkanDevice, vulkanSwapchain);
        commandManager.initialize(vulkanDevice, MAX_FRAMES_IN_FLIGHT);
        bufferManager.initialize(vulkanDevice, commandManager);
        textureManager.initialize(vulkanDevice, commandManager, bufferManager);
        descriptorManager.initialize(vulkanDevice);
        textureManager.createDepthResources(vulkanSwapchain.getExtent(), depthImage, depthImageMemory, depthImageView);
        createFramebuffers();
        textureManager.createTextureFromFile(TEXTURE_PATH, textureImage, textureImageMemory, textureImageView);
        textureSampler = textureManager.createTextureSampler();
        loadModel();
        bufferManager.createVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
        bufferManager.createIndexBuffer(indices, indexBuffer, indexBufferMemory);
        bufferManager.createUniformBuffer(MAX_FRAMES_IN_FLIGHT, uniformBuffers, uniformBuffersMemory, uniformBuffersMapped);
        descriptorManager.createDescriptorPool(MAX_FRAMES_IN_FLIGHT);
        descriptorManager.createDescriptorSets(vulkanPipeline.getDescriptorSetLayout(), MAX_FRAMES_IN_FLIGHT, uniformBuffers, textureImageView, textureSampler, descriptorSets);
        createSyncObjects();
    }

    void createSyncObjects(){
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
            VkResult imageAvailableSemaphoreResult = vkCreateSemaphore(vulkanDevice.getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            VkResult renderFinishedSemaphoreResult = vkCreateSemaphore(vulkanDevice.getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
            VkResult inFlightFenceResult = vkCreateFence(vulkanDevice.getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]);

            if(imageAvailableSemaphoreResult != VK_SUCCESS){
                std::cout<< "failed to create imageAvailableSemaphore for frame " << i << " - " << imageAvailableSemaphoreResult << std::endl;
                throw std::runtime_error("failed to create imageAvailableSemaphore!");
            }else{
                std::cout<< "Successfully created imageAvailableSemaphore for frame " << i << " - " << imageAvailableSemaphoreResult << std::endl;
            }

            if(renderFinishedSemaphoreResult != VK_SUCCESS){
                std::cout<< "failed to create renderFinishedSemaphore for frame " << i << " - " << renderFinishedSemaphoreResult << std::endl;
                throw std::runtime_error("failed to create renderFinishedSemaphore!");
            }else{
                std::cout<< "Successfully created renderFinishedSemaphore for frame " << i << " - " << renderFinishedSemaphoreResult << std::endl;
            }

            if(inFlightFenceResult != VK_SUCCESS){
                std::cout<< "failed to create inFlightFence for frame " << i << " - " << inFlightFenceResult << std::endl;
                throw std::runtime_error("failed to create inFlightFence!");
            }else{
                std::cout<< "Successfully created inFlightFence for frame " << i << " - " << inFlightFenceResult << std::endl;
            }
        }
    }

    void loadModel(){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                vertices.push_back(vertex);
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void createFramebuffers(){
        const std::vector<VkImageView>& swapChainImageViews = vulkanSwapchain.getImageViews();
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = vulkanPipeline.getRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = vulkanSwapchain.getExtent().width;
            framebufferInfo.height = vulkanSwapchain.getExtent().height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(vulkanDevice.getLogicalDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]);
            if (result != VK_SUCCESS) {
                std::cout<< "failed to create framebuffer - " << i << " - " << result <<std::endl;
                throw std::runtime_error("failed to create framebuffer!");
            }else{
                std::cout<< "successfully created framebuffer - " << i << " - " << result <<std::endl;
            }
        }
    }
    
    void createSurface() {
        if (glfwCreateWindowSurface(vulkanInstance.getInstance(), window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(vulkanDevice.getLogicalDevice());
    }

    void drawFrame() {
        vkWaitForFences(vulkanDevice.getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(vulkanDevice.getLogicalDevice(), 1, &inFlightFences[currentFrame]);

        uint32_t imageIndex;
        VkResult acquireNextImageResult = 
            vkAcquireNextImageKHR(vulkanDevice.getLogicalDevice(), vulkanSwapchain.getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (acquireNextImageResult != VK_SUCCESS && acquireNextImageResult != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        vkResetFences(vulkanDevice.getLogicalDevice(), 1, &inFlightFences[currentFrame]);
        commandManager.resetCommandBuffer(currentFrame);
        commandManager.recordCommandBuffer(
            commandManager.getCommandBuffer(currentFrame), 
            imageIndex,
            vulkanPipeline.getRenderPass(),
            swapChainFramebuffers[imageIndex],
            vulkanSwapchain.getExtent(),
            vulkanPipeline.getGraphicsPipeline(),
            vulkanPipeline.getPipelineLayout(),
            vertexBuffer,
            indexBuffer,
            descriptorSets,
            currentFrame,
            static_cast<uint32_t>(indices.size())
        );
        updateUniformBuffer(currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        VkCommandBuffer currentCommandBuffer = commandManager.getCommandBuffer(currentFrame);
        submitInfo.pCommandBuffers = &currentCommandBuffer;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult result = vkQueueSubmit(vulkanDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);
        if (result != VK_SUCCESS) {
            std::cout << "failed to submit draw command buffer - " << result << std::endl;
            throw std::runtime_error("failed to submit draw command buffer!");
        }else{
            // std::cout << "Successfully submmited command buffer - " << result << std::endl;
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {vulkanSwapchain.getSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VkResult queuePresentResult = vkQueuePresentKHR(vulkanDevice.getPresentQueue(), &presentInfo);
        if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (queuePresentResult != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(vulkanDevice.getLogicalDevice());

        vulkanSwapchain.recreate(window);
        textureManager.createDepthResources(vulkanSwapchain.getExtent(), depthImage, depthImageMemory, depthImageView);
        createFramebuffers();
    }

    void cleanup() {
        textureManager.destroyImageView(depthImageView);
        textureManager.destroyImage(depthImage, depthImageMemory);
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(vulkanDevice.getLogicalDevice(), framebuffer, nullptr);
        }
        textureManager.destroySampler(textureSampler);
        textureManager.destroyImageView(textureImageView);
        textureManager.destroyImage(textureImage, textureImageMemory);

        bufferManager.destroyBuffer(indexBuffer, indexBufferMemory);
        bufferManager.destroyBuffer(vertexBuffer, vertexBufferMemory);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            bufferManager.destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
        }
        descriptorManager.destroyDescriptorPool();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(vulkanDevice.getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanDevice.getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(vulkanDevice.getLogicalDevice(), inFlightFences[i], nullptr);
        }
        vkDestroySurfaceKHR(vulkanInstance.getInstance(), surface, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}