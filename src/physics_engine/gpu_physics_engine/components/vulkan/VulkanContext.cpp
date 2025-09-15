#include "VulkanContext.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include <iostream>

VulkanContext::VulkanContext() {
}

VulkanContext::~VulkanContext() {
    cleanup();
}

bool VulkanContext::initialize() {
    // Create and initialize Vulkan instance
    vulkanInstance = std::make_shared<VulkanInstance>();
    if (!vulkanInstance->initialize()) {
        std::cerr << "Failed to initialize Vulkan instance!" << std::endl;
        return false;
    }
    
    // Create and initialize Vulkan device
    vulkanDevice = std::make_shared<VulkanDevice>(vulkanInstance->getInstance());
    if (!vulkanDevice->initialize()) {
        std::cerr << "Failed to initialize Vulkan device!" << std::endl;
        return false;
    }
    
    // Create and initialize command pool
    vulkanCommandPool = std::make_shared<VulkanCommandPool>(vulkanDevice);
    if (!vulkanCommandPool->initialize()) {
        std::cerr << "Failed to initialize Vulkan command pool!" << std::endl;
        return false;
    }
    
    std::cout << "Vulkan context initialized successfully!" << std::endl;
    return true;
}

void VulkanContext::cleanup() {
    vulkanCommandPool.reset();
    vulkanDevice.reset();
    vulkanInstance.reset();
}

// Convenience accessors
VkInstance VulkanContext::getInstance() const {
    return vulkanInstance ? vulkanInstance->getInstance() : VK_NULL_HANDLE;
}

VkPhysicalDevice VulkanContext::getPhysicalDevice() const {
    return vulkanDevice ? vulkanDevice->getPhysicalDevice() : VK_NULL_HANDLE;
}

VkDevice VulkanContext::getDevice() const {
    return vulkanDevice ? vulkanDevice->getDevice() : VK_NULL_HANDLE;
}

VkQueue VulkanContext::getComputeQueue() const {
    return vulkanDevice ? vulkanDevice->getComputeQueue() : VK_NULL_HANDLE;
}

uint32_t VulkanContext::getComputeQueueFamily() const {
    return vulkanDevice ? vulkanDevice->getComputeQueueFamily() : 0;
}

VkCommandPool VulkanContext::getCommandPool() const {
    return vulkanCommandPool ? vulkanCommandPool->getCommandPool() : VK_NULL_HANDLE;
}