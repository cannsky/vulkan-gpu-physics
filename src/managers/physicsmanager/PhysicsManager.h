#pragma once

#include "../BaseManager.h"
#include "../../components/physics/Particle.h"
#include <vector>
#include <memory>

// Forward declarations
class VulkanManager;
class ParticleManager;
class CollisionManager;
class PhysicsLayerWorker;
class RigidBodyWorker;

/**
 * Central physics management system.
 * Coordinates all physics subsystems using worker-based approach.
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
    
    // Worker accessors
    std::shared_ptr<PhysicsLayerWorker> getLayerWorker() const { return layerWorker; }
    std::shared_ptr<RigidBodyWorker> getRigidBodyWorker() const { return rigidBodyWorker; }
    
    // Subsystem accessors
    std::shared_ptr<ParticleManager> getParticleManager() const;
    std::shared_ptr<CollisionManager> getCollisionManager() const;

private:
    PhysicsManager() = default;
    ~PhysicsManager() = default;
    
    bool initialized = false;
    uint32_t maxParticles = 1024;
    
    std::shared_ptr<PhysicsLayerWorker> layerWorker;
    std::shared_ptr<RigidBodyWorker> rigidBodyWorker;
    
    struct {
        float x = 0.0f;
        float y = -9.81f; 
        float z = 0.0f;
    } gravity;
};