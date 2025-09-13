#pragma once

#include "../BaseManager.h"
#include "../../components/physics/Particle.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

// Forward declarations
class VulkanContext;
class BufferManager;

/**
 * Manager for particle physics system.
 * Provides singleton access to particle operations.
 * Integrates all particle system functionality directly.
 */
class ParticleManager : public BaseManager {
public:
    static ParticleManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Particle operations
    bool addParticle(const Particle& particle);
    std::vector<Particle> getParticles() const;
    uint32_t getParticleCount() const;
    uint32_t getMaxParticles() const;
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // GPU operations
    void uploadParticlesToGPU();
    void downloadParticlesFromGPU();
    void updateUniformBuffer(float deltaTime);

private:
    ParticleManager() = default;
    ~ParticleManager() = default;
    
    // Initialization
    bool initializeInternal(uint32_t maxParticles = 1024);
    
    // Utility functions
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    bool initialized = false;
    
    // Vulkan context
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<BufferManager> bufferManager;
    
    // Particle data
    std::vector<Particle> particles;
    uint32_t maxParticles = 1024;
    
    // Uniform buffer object for GPU
    struct UniformBufferObject {
        float deltaTime;
        float gravity[3];
        uint32_t particleCount;
    } ubo;
};