#pragma once

#include "Particle.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

// Forward declarations
class VulkanContext;
class BufferManager;
class ComputePipeline;

class PhysicsEngine {
public:
    PhysicsEngine(std::shared_ptr<VulkanContext> context);
    ~PhysicsEngine();
    
    bool initialize(uint32_t maxParticles = 1024);
    void cleanup();
    
    bool addParticle(const Particle& particle);
    void updatePhysics(float deltaTime);
    std::vector<Particle> getParticles() const;
    
    void setGravity(float x, float y, float z);
    
    // Component accessors for advanced usage
    std::shared_ptr<BufferManager> getBufferManager() const { return bufferManager; }
    std::shared_ptr<ComputePipeline> getComputePipeline() const { return computePipeline; }

private:
    void recordComputeCommandBuffer();
    
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<BufferManager> bufferManager;
    std::shared_ptr<ComputePipeline> computePipeline;
    
    VkCommandBuffer computeCommandBuffer = VK_NULL_HANDLE;
    uint32_t maxParticles;
};