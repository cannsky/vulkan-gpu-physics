#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> computeFamily;
    
    bool isComplete() {
        return computeFamily.has_value();
    }
};

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();
    
    bool initialize();
    void cleanup();
    
    VkInstance getInstance() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getComputeQueue() const { return computeQueue; }
    uint32_t getComputeQueueFamily() const { return queueFamilyIndices.computeFamily.value(); }
    VkCommandPool getCommandPool() const { return commandPool; }

private:
    bool createInstance();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createCommandPool();
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    
    QueueFamilyIndices queueFamilyIndices;
    
    const std::vector<const char*> deviceExtensions = {};
    
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    bool checkValidationLayerSupport();
};