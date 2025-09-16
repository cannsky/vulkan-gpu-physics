#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <iostream>

class VulkanContext;
class BufferManager;

class GPUBufferWorker {
public:
    GPUBufferWorker(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager);
    ~GPUBufferWorker();
    
    // Initialize GPU buffers
    bool initialize(uint32_t maxContacts = 1024);
    void cleanup();
    
    // GPU buffer management
    void uploadContactsToGPU(uint32_t contactCount);
    void downloadContactsFromGPU();

private:
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<BufferManager> bufferManager;
    
    uint32_t maxContacts;
    
    // GPU compute resources
    VkBuffer contactBuffer = VK_NULL_HANDLE;
    VkDeviceMemory contactBufferMemory = VK_NULL_HANDLE;
    VkBuffer collisionPairBuffer = VK_NULL_HANDLE;
    VkDeviceMemory collisionPairBufferMemory = VK_NULL_HANDLE;
};