#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();
    
    bool initialize();
    void cleanup();
    
    VkInstance getInstance() const { return instance; }
    bool areValidationLayersEnabled() const { return enableValidationLayers; }

private:
    bool createInstance();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    
    VkInstance instance = VK_NULL_HANDLE;
    
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
};