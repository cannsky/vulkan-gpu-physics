#pragma once

#include <vulkan/vulkan.h>
#include <memory>

class VulkanDevice;

class VulkanCommandPool {
public:
    VulkanCommandPool(std::shared_ptr<VulkanDevice> device);
    ~VulkanCommandPool();
    
    bool initialize();
    void cleanup();
    
    VkCommandPool getCommandPool() const { return commandPool; }

private:
    std::shared_ptr<VulkanDevice> vulkanDevice;
    VkCommandPool commandPool = VK_NULL_HANDLE;
};