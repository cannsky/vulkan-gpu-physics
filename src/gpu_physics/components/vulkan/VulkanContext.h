#pragma once

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanInstance;
class VulkanDevice;
class VulkanCommandPool;

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();
    
    bool initialize();
    void cleanup();
    
    // Convenience accessors
    VkInstance getInstance() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkDevice getDevice() const;
    VkQueue getComputeQueue() const;
    uint32_t getComputeQueueFamily() const;
    VkCommandPool getCommandPool() const;
    
    // Component accessors
    std::shared_ptr<VulkanInstance> getVulkanInstance() const { return vulkanInstance; }
    std::shared_ptr<VulkanDevice> getVulkanDevice() const { return vulkanDevice; }
    std::shared_ptr<VulkanCommandPool> getVulkanCommandPool() const { return vulkanCommandPool; }

private:
    std::shared_ptr<VulkanInstance> vulkanInstance;
    std::shared_ptr<VulkanDevice> vulkanDevice;
    std::shared_ptr<VulkanCommandPool> vulkanCommandPool;
};