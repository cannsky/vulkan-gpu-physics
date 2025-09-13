#include "BufferManager.h"
#include "../VulkanContext.h"
#include "../Particle.h"
#include <iostream>
#include <cstring>

BufferManager::BufferManager(std::shared_ptr<VulkanContext> context) 
    : vulkanContext(context) {
}

BufferManager::~BufferManager() {
    cleanup();
}

bool BufferManager::initialize(uint32_t maxParticles) {
    if (!createBuffers(maxParticles)) {
        std::cerr << "Failed to create buffers!" << std::endl;
        return false;
    }
    
    std::cout << "Buffer manager initialized successfully!" << std::endl;
    return true;
}

void BufferManager::cleanup() {
    if (particleBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vulkanContext->getDevice(), particleBuffer, nullptr);
        particleBuffer = VK_NULL_HANDLE;
    }
    
    if (particleBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vulkanContext->getDevice(), particleBufferMemory, nullptr);
        particleBufferMemory = VK_NULL_HANDLE;
    }
    
    if (uniformBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vulkanContext->getDevice(), uniformBuffer, nullptr);
        uniformBuffer = VK_NULL_HANDLE;
    }
    
    if (uniformBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vulkanContext->getDevice(), uniformBufferMemory, nullptr);
        uniformBufferMemory = VK_NULL_HANDLE;
    }
}

bool BufferManager::createBuffers(uint32_t maxParticles) {
    VkDeviceSize particleBufferSize = sizeof(Particle) * maxParticles;
    
    // Create particle buffer
    if (!createBuffer(particleBufferSize, 
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     particleBuffer, particleBufferMemory)) {
        std::cerr << "Failed to create particle buffer!" << std::endl;
        return false;
    }
    
    // Create uniform buffer
    struct UniformBufferObject {
        float deltaTime;
        float gravity[3];
        uint32_t particleCount;
    };
    
    VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);
    
    if (!createBuffer(uniformBufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniformBuffer, uniformBufferMemory)) {
        std::cerr << "Failed to create uniform buffer!" << std::endl;
        return false;
    }
    
    return true;
}

bool BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                                VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                                VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanContext->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        std::cerr << "Failed to create buffer!" << std::endl;
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanContext->getDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate buffer memory!" << std::endl;
        return false;
    }

    vkBindBufferMemory(vulkanContext->getDevice(), buffer, bufferMemory, 0);
    return true;
}

uint32_t BufferManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}