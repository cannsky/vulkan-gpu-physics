#include "ParticleSystem.h"
#include "../physics/components/BufferManager.h"
#include "../physics/VulkanContext.h"
#include <iostream>
#include <cstring>

ParticleSystem::ParticleSystem(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager)
    : vulkanContext(context), bufferManager(bufferManager), maxParticles(0) {
    // Initialize gravity to Earth's gravity
    ubo.gravity[0] = 0.0f;
    ubo.gravity[1] = -9.81f;
    ubo.gravity[2] = 0.0f;
}

ParticleSystem::~ParticleSystem() {
    cleanup();
}

bool ParticleSystem::initialize(uint32_t maxParticles) {
    this->maxParticles = maxParticles;
    particles.reserve(maxParticles);
    
    std::cout << "Particle system initialized with " << maxParticles << " max particles" << std::endl;
    return true;
}

void ParticleSystem::cleanup() {
    particles.clear();
}

bool ParticleSystem::addParticle(const Particle& particle) {
    if (particles.size() >= maxParticles) {
        std::cerr << "Cannot add particle: maximum capacity reached!" << std::endl;
        return false;
    }
    
    particles.push_back(particle);
    return true;
}

std::vector<Particle> ParticleSystem::getParticles() const {
    return particles;
}

void ParticleSystem::setGravity(float x, float y, float z) {
    ubo.gravity[0] = x;
    ubo.gravity[1] = y;
    ubo.gravity[2] = z;
}

void ParticleSystem::updateUniformBuffer(float deltaTime) {
    ubo.deltaTime = deltaTime;
    ubo.particleCount = static_cast<uint32_t>(particles.size());
    
    void* data;
    vkMapMemory(vulkanContext->getDevice(), bufferManager->getUniformBufferMemory(), 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(vulkanContext->getDevice(), bufferManager->getUniformBufferMemory());
}

void ParticleSystem::uploadParticlesToGPU() {
    if (particles.empty()) {
        return;
    }
    
    VkDeviceSize bufferSize = sizeof(Particle) * particles.size();
    
    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanContext->getDevice(), &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create staging buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanContext->getDevice(), stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate staging buffer memory!");
    }

    vkBindBufferMemory(vulkanContext->getDevice(), stagingBuffer, stagingBufferMemory, 0);

    void* data;
    vkMapMemory(vulkanContext->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, particles.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(vulkanContext->getDevice(), stagingBufferMemory);

    copyBuffer(stagingBuffer, bufferManager->getParticleBuffer(), bufferSize);

    vkDestroyBuffer(vulkanContext->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanContext->getDevice(), stagingBufferMemory, nullptr);
}

void ParticleSystem::downloadParticlesFromGPU() {
    if (particles.empty()) {
        return;
    }
    
    VkDeviceSize bufferSize = sizeof(Particle) * particles.size();
    
    // Create staging buffer for download
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanContext->getDevice(), &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create staging buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanContext->getDevice(), stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate staging buffer memory!");
    }

    vkBindBufferMemory(vulkanContext->getDevice(), stagingBuffer, stagingBufferMemory, 0);

    copyBuffer(bufferManager->getParticleBuffer(), stagingBuffer, bufferSize);

    void* data;
    vkMapMemory(vulkanContext->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(particles.data(), data, static_cast<size_t>(bufferSize));
    vkUnmapMemory(vulkanContext->getDevice(), stagingBufferMemory);

    vkDestroyBuffer(vulkanContext->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanContext->getDevice(), stagingBufferMemory, nullptr);
}

void ParticleSystem::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanContext->getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanContext->getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkanContext->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanContext->getComputeQueue());

    vkFreeCommandBuffers(vulkanContext->getDevice(), vulkanContext->getCommandPool(), 1, &commandBuffer);
}

uint32_t ParticleSystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}