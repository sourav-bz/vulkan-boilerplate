#include "CommandManager.h"
#include "../core/VulkanDevice.h"
#include <iostream>
#include <stdexcept>
#include <array>

CommandManager::CommandManager(){
}

CommandManager::~CommandManager(){
    cleanup();
}

void CommandManager::initialize(const VulkanDevice& device, uint32_t maxFramesInFlight){
    this->vulkanDevice = &device;

    createCommandPool();
    createCommandBuffers(maxFramesInFlight);
}

void CommandManager::cleanup(){
    if(vulkanDevice && commandPool != VK_NULL_HANDLE){
        vkDestroyCommandPool(vulkanDevice->getLogicalDevice(), commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
}

void CommandManager::createCommandPool(){
    QueueFamilyIndices queueFamilyIndices = vulkanDevice->findQueueFamilies(vulkanDevice->getPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkResult result = vkCreateCommandPool(vulkanDevice->getLogicalDevice(), &poolInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
        std::cout<< "failed to create command pool - " << result <<std::endl;
        throw std::runtime_error("failed to create command pool!");
    }else{
        std::cout<< "Successfully created command pool - " << result <<std::endl;
    }
}

void CommandManager::createCommandBuffers(uint32_t maxFramesInFlight){
    commandBuffers.resize(maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    VkResult result = vkAllocateCommandBuffers(vulkanDevice->getLogicalDevice(), &allocInfo, commandBuffers.data());
    if (result != VK_SUCCESS) {
        std::cout<< "failed to create command buffer - " << result <<std::endl;
        throw std::runtime_error("failed to allocate command buffers!");
    }else{
        std::cout<< "successfully created command buffer - " << result <<std::endl;
    }
}

VkCommandBuffer CommandManager::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanDevice->getLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CommandManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkanDevice->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanDevice->getGraphicsQueue());

    vkFreeCommandBuffers(vulkanDevice->getLogicalDevice(), commandPool, 1, &commandBuffer);
}

void CommandManager::resetCommandBuffer(uint32_t frameIndex) {
    vkResetCommandBuffer(commandBuffers[frameIndex], 0);
}

void CommandManager::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, 
                           VkRenderPass renderPass, VkFramebuffer framebuffer,
                           VkExtent2D extent, VkPipeline graphicsPipeline,
                           VkPipelineLayout pipelineLayout, VkBuffer vertexBuffer,
                           VkBuffer indexBuffer, const std::vector<VkDescriptorSet>& descriptorSets,
                           uint32_t currentFrame, uint32_t indexCount) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    VkResult beginCommandBufferResult = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if ( beginCommandBufferResult != VK_SUCCESS) {
        std::cout << "failed to begin recording command buffer - " << beginCommandBufferResult << std::endl;
        throw std::runtime_error("failed to begin recording command buffer!");
    }else{
        //std::cout << "Successfully started recording command buffer - " << beginCommandBufferResult << std::endl;
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexCount), 1, 0, 0, 0);
}
