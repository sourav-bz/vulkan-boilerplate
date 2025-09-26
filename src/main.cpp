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

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cstdint>
#include <array>
#include <unordered_map>

#include "common/Vertex.h"
#include "common/VertexTypes.h"
#include "core/VulkanApplication.h"
#include "rendering/VulkanGraphicsPipeline.h"
#include "rendering/CommandManager.h"
#include "resources/BufferManager.h"
#include "resources/TextureManager.h"
#include "descriptors/DescriptorManager.h"
#include "ui/GuiManager.h"

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

class MyVulkanApp : public VulkanApplication {

private:
    std::vector<StandardVertex> vertices_;
    std::vector<uint32_t> indices_;
    VkBuffer vertexBuffer_;
    VkDeviceMemory vertexBufferMemory_;
    VkBuffer indexBuffer_;
    VkDeviceMemory indexBufferMemory_;
    std::vector<VkBuffer> uniformBuffers_;
    std::vector<VkDeviceMemory> uniformBuffersMemory_;
    std::vector<void*> uniformBuffersMapped_;
    std::vector<VkDescriptorSet> descriptorSets_;
    VkImage textureImage_;
    VkDeviceMemory textureImageMemory_;
    VkImageView textureImageView_;
    VkSampler textureSampler_;
    VkImage depthImage_;
    VkDeviceMemory depthImageMemory_;
    VkImageView depthImageView_;
    std::vector<VkFramebuffer> swapChainFramebuffers_;

    // GUI state variables (from the ImGui example)
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

public:
    MyVulkanApp() : VulkanApplication({
        .windowWidth = 1280,
        .windowHeight = 800,
        .windowTitle = "Vulkan Boilerplate with ImGui",
        .maxFramesInFlight = 2,
        .enableValidation = true,
        .enableGui = true  // Enable GUI
    }) {}

protected:
    // Override the GUI rendering method
    void renderGui() override {
        // This is the GUI content from the ImGui example
        
        // 1. Show the big demo window
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");

            ImGui::Text("This is some useful text.");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);

            if (ImGui::Button("Button"))
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                       1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window
        if (show_another_window) {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
    }

    void initializeResources() override {
        textureManager_->createDepthResources(vulkanSwapchain_->getExtent(), depthImage_, depthImageMemory_, depthImageView_);
        createFramebuffers();
        textureManager_->createTextureFromFile(TEXTURE_PATH, textureImage_, textureImageMemory_, textureImageView_);
        textureSampler_ = textureManager_->createTextureSampler();

        loadModel();

        bufferManager_->createVertexBuffer(vertices_, vertexBuffer_, vertexBufferMemory_);
        bufferManager_->createIndexBuffer(indices_, indexBuffer_, indexBufferMemory_);
        bufferManager_->createUniformBuffer(config_.maxFramesInFlight, uniformBuffers_, uniformBuffersMemory_, uniformBuffersMapped_);

        descriptorManager_->createDescriptorPool(config_.maxFramesInFlight);
        descriptorManager_->createDescriptorSets(vulkanPipeline_->getDescriptorSetLayout(), 
                                                    config_.maxFramesInFlight, 
                                                    uniformBuffers_, 
                                                    textureImageView_, 
                                                    textureSampler_, 
                                                    descriptorSets_);
    }

    void updateUniforms(uint32_t currentImage) override {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkanSwapchain_->getExtent().width / (float) vulkanSwapchain_->getExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffersMapped_[currentImage], &ubo, sizeof(ubo));
    }

    void recordRenderCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex) override {
        commandManager_->resetCommandBuffer(currentFrame_);
        commandManager_->recordCommandBuffer(
            commandBuffer, 
            imageIndex,
            vulkanPipeline_->getRenderPass(),
            swapChainFramebuffers_[imageIndex],
            vulkanSwapchain_->getExtent(),
            vulkanPipeline_->getGraphicsPipeline(),
            vulkanPipeline_->getPipelineLayout(),
            vertexBuffer_,
            indexBuffer_,
            descriptorSets_,
            currentFrame_,
            static_cast<uint32_t>(indices_.size())
        );
        
        // Render GUI if enabled (still within the render pass)
        if (config_.enableGui && guiManager_) {
            guiManager_->render(commandBuffer);
        }
        
        // End render pass and command buffer
        vkCmdEndRenderPass(commandBuffer);
        
        VkResult result = vkEndCommandBuffer(commandBuffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to end command buffer recording!");
        }
    }
    
    void onCleanup() override {
        textureManager_->destroyImageView(depthImageView_);
        textureManager_->destroyImage(depthImage_, depthImageMemory_);
        
        for (auto framebuffer : swapChainFramebuffers_) {
            vkDestroyFramebuffer(vulkanDevice_->getLogicalDevice(), framebuffer, nullptr);
        }
        
        textureManager_->destroySampler(textureSampler_);
        textureManager_->destroyImageView(textureImageView_);
        textureManager_->destroyImage(textureImage_, textureImageMemory_);

        bufferManager_->destroyBuffer(indexBuffer_, indexBufferMemory_);
        bufferManager_->destroyBuffer(vertexBuffer_, vertexBufferMemory_);
        for (size_t i = 0; i < config_.maxFramesInFlight; i++) {
            bufferManager_->destroyBuffer(uniformBuffers_[i], uniformBuffersMemory_[i]);
        }
        
        descriptorManager_->destroyDescriptorPool();
    }

private:
    void loadModel() {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(err);
        }

        std::unordered_map<StandardVertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                StandardVertex vertex{};

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

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices_.size());
                    vertices_.push_back(vertex);
                }

                indices_.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void createFramebuffers(){
        const std::vector<VkImageView>& swapChainImageViews = vulkanSwapchain_->getImageViews();
        swapChainFramebuffers_.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView_
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = vulkanPipeline_->getRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = vulkanSwapchain_->getExtent().width;
            framebufferInfo.height = vulkanSwapchain_->getExtent().height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(vulkanDevice_->getLogicalDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers_[i]);
            if (result != VK_SUCCESS) {
                std::cout<< "failed to create framebuffer - " << i << " - " << result <<std::endl;
                throw std::runtime_error("failed to create framebuffer!");
            }else{
                std::cout<< "successfully created framebuffer - " << i << " - " << result <<std::endl;
            }
        }
    }
};

int main() {
    try {
        MyVulkanApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}