#pragma once

#include "VulkanContext.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

struct Particle {
    float position[3];
    float velocity[3];
    float mass;
    float padding; // For alignment
};

class VulkanPhysics {
public:
    VulkanPhysics(std::shared_ptr<VulkanContext> context);
    ~VulkanPhysics();
    
    bool initialize(uint32_t maxParticles = 1024);
    void cleanup();
    
    bool addParticle(const Particle& particle);
    void updatePhysics(float deltaTime);
    std::vector<Particle> getParticles() const;
    
    void setGravity(float x, float y, float z);
    
private:
    bool createBuffers(uint32_t maxParticles);
    bool createComputePipeline();
    bool createDescriptorSets();
    bool loadShader(const std::string& filename, std::vector<char>& shaderCode);
    
    void updateUniformBuffer(float deltaTime);
    void recordComputeCommandBuffer();
    
    std::shared_ptr<VulkanContext> vulkanContext;
    
    // Vulkan objects
    VkBuffer particleBuffer = VK_NULL_HANDLE;
    VkDeviceMemory particleBufferMemory = VK_NULL_HANDLE;
    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
    
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline computePipeline = VK_NULL_HANDLE;
    
    VkCommandBuffer computeCommandBuffer = VK_NULL_HANDLE;
    
    // Physics data
    std::vector<Particle> particles;
    uint32_t maxParticles;
    
    struct UniformBufferObject {
        float deltaTime;
        float gravity[3];
        uint32_t particleCount;
    } ubo;
    
    // Helper functions
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                     VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                     VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};