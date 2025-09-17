#pragma once

#include "../Contact.h"
#include "../../../../CPUPhysicsEngine/entities/RigidBody.h"
#include <vector>

class BroadPhaseWorker {
public:
    BroadPhaseWorker() = default;
    ~BroadPhaseWorker() = default;
    
    // Update broad phase collision pairs
    void updateBroadPhase(const std::vector<RigidBody>& rigidBodies, std::vector<CollisionPair>& collisionPairs);

private:
    // Simple AABB check for broad phase
    bool isAABBOverlapping(const RigidBody& bodyA, const RigidBody& bodyB) const;
    
    // Get conservative bounding sphere radius
    float getMaxRadius(const RigidBody& body) const;
};