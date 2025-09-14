#pragma once

#include "../components/physics/Particle.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

// Forward declarations
class VulkanContext;
class BufferManager;
class ComputePipeline;

namespace gpu_physics {

/**
 * GPU Physics System - Handles particle and fluid simulations
 * Uses Vulkan compute shaders for high-performance GPU-accelerated physics
 */
class GPUPhysicsSystem {
public:
    GPUPhysicsSystem(std::shared_ptr<VulkanContext> context);
    ~GPUPhysicsSystem();
    
    bool initialize(uint32_t maxParticles = 1024);
    void cleanup();
    
    // Particle management
    bool addParticle(const Particle& particle);
    bool addParticle(float x, float y, float z, float vx = 0.0f, float vy = 0.0f, float vz = 0.0f, float mass = 1.0f);
    std::vector<Particle> getParticles() const;
    size_t getParticleCount() const;
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // Configuration
    uint32_t getMaxParticles() const { return maxParticles; }
    
    // GPU buffer management
    void uploadParticlesToGPU();
    void downloadParticlesFromGPU();
    
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
    
    std::vector<Particle> particles;
    
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;
};

} // namespace gpu_physics