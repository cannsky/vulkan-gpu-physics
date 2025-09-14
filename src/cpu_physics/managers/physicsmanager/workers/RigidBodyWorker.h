#pragma once

#include "../../../../components/rigidbody/RigidBody.h"
#include <memory>
#include <vector>

/**
 * RigidBody worker for PhysicsManager.
 * Manages rigidbody operations with CPU-based physics simulation.
 */
class RigidBodyWorker {
public:
    RigidBodyWorker();
    ~RigidBodyWorker() = default;
    
    // Initialize the worker
    bool initialize();
    void cleanup();
    
    // RigidBody management
    uint32_t addRigidBody(const RigidBody& body);
    bool removeRigidBody(uint32_t bodyId);
    RigidBody* getRigidBody(uint32_t bodyId);
    
    // Physics operations
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // Configuration
    void setMaxRigidBodies(uint32_t maxBodies) { maxRigidBodies = maxBodies; }
    uint32_t getMaxRigidBodies() const { return maxRigidBodies; }
    uint32_t getRigidBodyCount() const { return static_cast<uint32_t>(rigidBodies.size()); }

private:
    bool initialized = false;
    uint32_t maxRigidBodies = 512;
    uint32_t nextId = 1;
    
    std::vector<RigidBody> rigidBodies;
    std::vector<uint32_t> freeIds; // For ID reuse
    
    struct {
        float x = 0.0f;
        float y = -9.81f; 
        float z = 0.0f;
    } gravity;
};