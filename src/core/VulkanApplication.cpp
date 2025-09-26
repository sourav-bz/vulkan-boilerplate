#include "VulkanApplication.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "../rendering/VulkanSwapchain.h"
#include "../rendering/VulkanGraphicsPipeline.h"
#include "../rendering/CommandManager.h"
#include "../resources/BufferManager.h"
#include "../resources/TextureManager.h"
#include "../descriptors/DescriptorManager.h"
#include <stdexcept>
#include <iostream>
#include <memory>

VulkanApplication::VulkanApplication(const Config& config)
    : config_(config), window_(nullptr), surface_(VK_NULL_HANDLE) {
}

VulkanApplication::~VulkanApplication(){
    cleanup();
}

void VulkanApplication::run(){
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanApplication::initWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ = glfwCreateWindow(config_.windowWidth, config_.windowHeight, 
                            config_.windowTitle.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
}

void VulkanApplication::initVulkan(){
    vulkanInstance_ = std::make_unique<VulkanInstance>();
    vulkanInstance_->initialize();

    if(glfwCreateWindowSurface(vulkanInstance_->getInstance(), window_, nullptr, &surface_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }

    vulkanDevice_ = std::make_unique<VulkanDevice>();
    vulkanDevice_->initialize(*vulkanInstance_, surface_);

    vulkanSwapchain_ = std::make_unique<VulkanSwapchain>();
    vulkanSwapchain_->initialize(*vulkanDevice_, surface_, window_);

    vulkanPipeline_ = std::make_unique<VulkanGraphicsPipeline>();
    vulkanPipeline_->initialize(*vulkanDevice_, *vulkanSwapchain_);

    commandManager_ = std::make_unique<CommandManager>();
    commandManager_->initialize(*vulkanDevice_, config_.maxFramesInFlight);

    bufferManager_ = std::make_unique<BufferManager>();
    bufferManager_->initialize(*vulkanDevice_, *commandManager_);

    textureManager_ = std::make_unique<TextureManager>();
    textureManager_->initialize(*vulkanDevice_, *commandManager_, *bufferManager_);

    descriptorManager_ = std::make_unique<DescriptorManager>();
    descriptorManager_->initialize(*vulkanDevice_);

    createSyncObjects();

    initializeResources();
}

void VulkanApplication::createSyncObjects() {
    imageAvailableSemaphores_.resize(config_.maxFramesInFlight);
    renderFinishedSemaphores_.resize(config_.maxFramesInFlight);
    inFlightFences_.resize(config_.maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < config_.maxFramesInFlight; i++) {
        if (vkCreateSemaphore(vulkanDevice_->getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkanDevice_->getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(vulkanDevice_->getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void VulkanApplication::mainLoop() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(vulkanDevice_->getLogicalDevice());
}

void VulkanApplication::drawFrame() {
    vkWaitForFences(vulkanDevice_->getLogicalDevice(), 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
    vkResetFences(vulkanDevice_->getLogicalDevice(), 1, &inFlightFences_[currentFrame_]);

    uint32_t imageIndex;
    VkResult acquireNextImageResult = 
        vkAcquireNextImageKHR(vulkanDevice_->getLogicalDevice(), vulkanSwapchain_->getSwapChain(), UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
    if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (acquireNextImageResult != VK_SUCCESS && acquireNextImageResult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    vkResetFences(vulkanDevice_->getLogicalDevice(), 1, &inFlightFences_[currentFrame_]);

    updateUniforms(currentFrame_);
    // Record command buffer - delegate to derived class
    recordRenderCommands(commandManager_->getCommandBuffer(currentFrame_), imageIndex);


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer currentCommandBuffer = commandManager_->getCommandBuffer(currentFrame_);
    submitInfo.pCommandBuffers = &currentCommandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult result = vkQueueSubmit(vulkanDevice_->getGraphicsQueue(), 1, &submitInfo, inFlightFences_[currentFrame_]);
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
    VkSwapchainKHR swapChains[] = {vulkanSwapchain_->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    VkResult queuePresentResult = vkQueuePresentKHR(vulkanDevice_->getPresentQueue(), &presentInfo);
    if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR || framebufferResized_) {
        framebufferResized_ = false;
        recreateSwapChain();
    } else if (queuePresentResult != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame_ = (currentFrame_ + 1) % config_.maxFramesInFlight;
}

void VulkanApplication::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window_, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window_, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(vulkanDevice_->getLogicalDevice());

    vulkanSwapchain_->recreate(window_);
}

void VulkanApplication::cleanup() {
    if (vulkanDevice_ && vulkanDevice_->getLogicalDevice()) {
        vkDeviceWaitIdle(vulkanDevice_->getLogicalDevice());
    }

    onCleanup();

    for (size_t i = 0; i < config_.maxFramesInFlight; i++) {
        if (vulkanDevice_ && vulkanDevice_->getLogicalDevice()) {
            if (renderFinishedSemaphores_[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(vulkanDevice_->getLogicalDevice(), renderFinishedSemaphores_[i], nullptr);
                renderFinishedSemaphores_[i] = VK_NULL_HANDLE;
            }
            if (imageAvailableSemaphores_[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(vulkanDevice_->getLogicalDevice(), imageAvailableSemaphores_[i], nullptr);
                imageAvailableSemaphores_[i] = VK_NULL_HANDLE;
            }
            if (inFlightFences_[i] != VK_NULL_HANDLE) {
                vkDestroyFence(vulkanDevice_->getLogicalDevice(), inFlightFences_[i], nullptr);
                inFlightFences_[i] = VK_NULL_HANDLE;
            }
        }
    }

    if (vulkanSwapchain_) {
        vulkanSwapchain_.reset();
    }

    if (surface_ != VK_NULL_HANDLE && vulkanInstance_) {
        vkDestroySurfaceKHR(vulkanInstance_->getInstance(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

void VulkanApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized_ = true;
}