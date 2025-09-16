#pragma once

#include "../../../managers/BaseManager.h"
#include <vector>
#include <memory>

// Forward declarations
class VulkanManager;
class ParticleManager;
class CollisionManager;

/**
 * GPU-side physics management system.
 * Handles particles, fluids and limited collision interactions with CPU physics.
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
    std::shared_ptr<ParticleManager> getParticleManager() const;
    std::shared_ptr<CollisionManager> getCollisionManager() const;

private:
    GPUPhysicsManager() = default;
    ~GPUPhysicsManager() = default;
    
    bool initialized = false;
    uint32_t maxParticles = 1024;
    
    struct {
        float x = 0.0f;
        float y = -9.81f; 
        float z = 0.0f;
    } gravity;
};