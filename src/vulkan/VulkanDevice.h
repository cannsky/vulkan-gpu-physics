#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <memory>

struct QueueFamilyIndices {
    std::optional<uint32_t> computeFamily;
    
    bool isComplete() {
        return computeFamily.has_value();
    }
};

class VulkanDevice {
public:
    VulkanDevice(VkInstance instance);
    ~VulkanDevice();
    
    bool initialize();
    void cleanup();
    
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getComputeQueue() const { return computeQueue; }
    uint32_t getComputeQueueFamily() const { return queueFamilyIndices.computeFamily.value(); }
    const QueueFamilyIndices& getQueueFamilyIndices() const { return queueFamilyIndices; }

private:
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    
    QueueFamilyIndices queueFamilyIndices;
    
    const std::vector<const char*> deviceExtensions = {};
};