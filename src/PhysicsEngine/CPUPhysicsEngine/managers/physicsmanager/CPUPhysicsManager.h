#pragma once

#include "../../../managers/BaseManager.h"
#include <vector>
#include <memory>

// Forward declarations
class PhysicsLayerWorker;
class RigidBodyWorker;

/**
 * CPU-side physics management system.
 * Handles rigidbodies and collision layers for CPU physics simulation.
 */
class CPUPhysicsManager : public BaseManager {
public:
    static CPUPhysicsManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // Worker accessors
    std::shared_ptr<PhysicsLayerWorker> getLayerWorker() const { return layerWorker; }
    std::shared_ptr<RigidBodyWorker> getRigidBodyWorker() const { return rigidBodyWorker; }

private:
    CPUPhysicsManager() = default;
    ~CPUPhysicsManager() = default;
    
    bool initialized = false;
    
    std::shared_ptr<PhysicsLayerWorker> layerWorker;
    std::shared_ptr<RigidBodyWorker> rigidBodyWorker;
    
    struct {
        float x = 0.0f;
        float y = -9.81f; 
        float z = 0.0f;
    } gravity;
};