#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include <iostream>

VulkanCommandPool::VulkanCommandPool(std::shared_ptr<VulkanDevice> device) 
    : vulkanDevice(device) {
}

VulkanCommandPool::~VulkanCommandPool() {
    cleanup();
}

bool VulkanCommandPool::initialize() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = vulkanDevice->getComputeQueueFamily();

    if (vkCreateCommandPool(vulkanDevice->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        std::cerr << "Failed to create command pool!" << std::endl;
        return false;
    }

    std::cout << "Vulkan command pool created successfully!" << std::endl;
    return true;
}

void VulkanCommandPool::cleanup() {
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(vulkanDevice->getDevice(), commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
}