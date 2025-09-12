#pragma once

#include "../physics/Particle.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class VulkanContext;
class BufferManager;

class ParticleSystem {
public:
    ParticleSystem(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager);
    ~ParticleSystem();
    
    bool initialize(uint32_t maxParticles);
    void cleanup();
    
    bool addParticle(const Particle& particle);
    std::vector<Particle> getParticles() const;
    void setGravity(float x, float y, float z);
    
    void updateUniformBuffer(float deltaTime);
    void uploadParticlesToGPU();
    void downloadParticlesFromGPU();
    
    uint32_t getParticleCount() const { return static_cast<uint32_t>(particles.size()); }
    uint32_t getMaxParticles() const { return maxParticles; }

private:
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<BufferManager> bufferManager;
    
    std::vector<Particle> particles;
    uint32_t maxParticles;
    
    struct UniformBufferObject {
        float deltaTime;
        float gravity[3];
        uint32_t particleCount;
    } ubo;
};