#pragma once

#include "../../../managers/BaseManager.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// Forward declarations
class VulkanInstance;
class VulkanDevice;
class VulkanCommandPool;
class VulkanContext;

/**
 * Central manager for all Vulkan operations.
 * Provides singleton access to Vulkan resources throughout the physics engine.
 */
class VulkanManager : public BaseManager {
public:
    static VulkanManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Vulkan resource accessors
    std::shared_ptr<VulkanContext> getContext() const { return vulkanContext; }
    std::shared_ptr<VulkanInstance> getVulkanInstance() const { return vulkanInstance; }
    std::shared_ptr<VulkanDevice> getDevice() const { return vulkanDevice; }
    std::shared_ptr<VulkanCommandPool> getCommandPool() const { return commandPool; }
    
    // Vulkan device properties
    VkDevice getLogicalDevice() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkQueue getComputeQueue() const;
    uint32_t getComputeQueueFamily() const;
    
    // Vulkan operations
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    
    // Memory management helpers
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:
    VulkanManager() = default;
    ~VulkanManager() = default;
    
    bool initialized = false;
    
    std::shared_ptr<VulkanInstance> vulkanInstance;
    std::shared_ptr<VulkanDevice> vulkanDevice;
    std::shared_ptr<VulkanCommandPool> commandPool;
    std::shared_ptr<VulkanContext> vulkanContext;
};