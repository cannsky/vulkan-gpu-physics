#include "VulkanManager.h"
#include "../../components/vulkan/VulkanInstance.h"
#include "../../components/vulkan/VulkanDevice.h"
#include "../../components/vulkan/VulkanCommandPool.h"
#include "../../components/vulkan/VulkanContext.h"
#include <stdexcept>

VulkanManager& VulkanManager::getInstance() {
    static VulkanManager instance;
    return instance;
}

bool VulkanManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Create Vulkan instance
        vulkanInstance = std::make_shared<VulkanInstance>();
        if (!vulkanInstance->initialize()) {
            cleanup();
            return false;
        }
        
        // Create Vulkan device (requires VkInstance)
        vulkanDevice = std::make_shared<VulkanDevice>(vulkanInstance->getInstance());
        if (!vulkanDevice->initialize()) {
            cleanup();
            return false;
        }
        
        // Create command pool (requires VulkanDevice)
        commandPool = std::make_shared<VulkanCommandPool>(vulkanDevice);
        if (!commandPool->initialize()) {
            cleanup();
            return false;
        }
        
        // Create Vulkan context wrapper
        vulkanContext = std::make_shared<VulkanContext>();
        if (!vulkanContext->initialize()) {
            cleanup();
            return false;
        }
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void VulkanManager::cleanup() {
    if (vulkanContext) {
        vulkanContext->cleanup();
        vulkanContext.reset();
    }
    
    if (commandPool) {
        commandPool->cleanup();
        commandPool.reset();
    }
    
    if (vulkanDevice) {
        vulkanDevice->cleanup();
        vulkanDevice.reset();
    }
    
    if (vulkanInstance) {
        vulkanInstance->cleanup();
        vulkanInstance.reset();
    }
    
    initialized = false;
}

bool VulkanManager::isInitialized() const {
    return initialized;
}

VkDevice VulkanManager::getLogicalDevice() const {
    if (!vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    return vulkanDevice->getDevice();
}

VkPhysicalDevice VulkanManager::getPhysicalDevice() const {
    if (!vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    return vulkanDevice->getPhysicalDevice();
}

VkQueue VulkanManager::getComputeQueue() const {
    if (!vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    return vulkanDevice->getComputeQueue();
}

uint32_t VulkanManager::getComputeQueueFamily() const {
    if (!vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    return vulkanDevice->getComputeQueueFamily();
}

VkCommandBuffer VulkanManager::beginSingleTimeCommands() {
    if (!commandPool || !vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool->getCommandPool();
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanDevice->getDevice(), &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

void VulkanManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    if (!commandPool || !vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(vulkanDevice->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanDevice->getComputeQueue());
    
    vkFreeCommandBuffers(vulkanDevice->getDevice(), commandPool->getCommandPool(), 1, &commandBuffer);
}

uint32_t VulkanManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    if (!vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkanDevice->getPhysicalDevice(), &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type");
}

void VulkanManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    if (!vulkanDevice) {
        throw std::runtime_error("VulkanManager not initialized");
    }
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(vulkanDevice->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanDevice->getDevice(), buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(vulkanDevice->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory");
    }
    
    vkBindBufferMemory(vulkanDevice->getDevice(), buffer, bufferMemory, 0);
}

void VulkanManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    endSingleTimeCommands(commandBuffer);
}