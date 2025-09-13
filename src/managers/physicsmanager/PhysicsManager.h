#pragma once

#include "../BaseManager.h"
#include "../../physics/Particle.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

// Forward declarations
class VulkanManager;
class BufferManager;
class ComputePipeline;
class ParticleManager;
class CollisionManager;

/**
 * Central physics management system.
 * Replaces PhysicsEngine with a singleton manager pattern.
 * Coordinates all physics subsystems.
 */
class PhysicsManager : public BaseManager {
public:
    static PhysicsManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // Configuration
    bool setMaxParticles(uint32_t maxParticles);
    uint32_t getMaxParticles() const { return maxParticles; }
    
    // Component accessors for advanced usage
    std::shared_ptr<BufferManager> getBufferManager() const { return bufferManager; }
    std::shared_ptr<ComputePipeline> getComputePipeline() const { return computePipeline; }
    
    // Subsystem accessors
    std::shared_ptr<ParticleManager> getParticleManager() const;
    std::shared_ptr<CollisionManager> getCollisionManager() const;

private:
    PhysicsManager() = default;
    ~PhysicsManager() = default;
    
    void recordComputeCommandBuffer();
    
    bool initialized = false;
    uint32_t maxParticles = 1024;
    
    std::shared_ptr<BufferManager> bufferManager;
    std::shared_ptr<ComputePipeline> computePipeline;
    
    VkCommandBuffer computeCommandBuffer = VK_NULL_HANDLE;
    
    struct {
        float x = 0.0f;
        float y = -9.81f; 
        float z = 0.0f;
    } gravity;
};