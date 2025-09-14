#pragma once

#include <vulkan/vulkan.h>
#include <memory>

class VulkanContext;

class BufferManager {
public:
    BufferManager(std::shared_ptr<VulkanContext> context);
    ~BufferManager();
    
    bool initialize(uint32_t maxParticles);
    void cleanup();
    
    VkBuffer getParticleBuffer() const { return particleBuffer; }
    VkDeviceMemory getParticleBufferMemory() const { return particleBufferMemory; }
    VkBuffer getUniformBuffer() const { return uniformBuffer; }
    VkDeviceMemory getUniformBufferMemory() const { return uniformBufferMemory; }

private:
    bool createBuffers(uint32_t maxParticles);
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                     VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                     VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    std::shared_ptr<VulkanContext> vulkanContext;
    
    VkBuffer particleBuffer = VK_NULL_HANDLE;
    VkDeviceMemory particleBufferMemory = VK_NULL_HANDLE;
    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
};