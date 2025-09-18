#pragma once

#include "../../../managers/BaseManager.h"
#include <vector>
#include <memory>

// Forward declarations
class VulkanManager;

namespace gpu_physics {
    class ECSManager;
    class ParticlePhysicsSystem;
}

/**
 * GPU-side physics management system.
 * Handles particles using ECS architecture for efficient GPU data management.
 * No longer handles collisions - particles only.
 */
class GPUPhysicsManager : public BaseManager {
public:
    static GPUPhysicsManager& getInstance();
    
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
    
    // Subsystem accessors
    std::shared_ptr<gpu_physics::ECSManager> getECSManager() const;
    std::shared_ptr<gpu_physics::ParticlePhysicsSystem> getParticlePhysicsSystem() const;

private:
    GPUPhysicsManager() = default;
    ~GPUPhysicsManager() = default;
    
    bool initialized = false;
    uint32_t maxParticles = 1024;
    
    // ECS-based particle system
    std::shared_ptr<gpu_physics::ECSManager> ecsManager;
    std::shared_ptr<gpu_physics::ParticlePhysicsSystem> particlePhysicsSystem;
    
    struct {
        float x = 0.0f;
        float y = -9.81f; 
        float z = 0.0f;
    } gravity;
};