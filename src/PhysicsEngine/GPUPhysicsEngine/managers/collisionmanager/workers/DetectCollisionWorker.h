#pragma once

#include "../Contact.h"
#include "../../../../CPUPhysicsEngine/entities/RigidBody.h"
#include <vector>
#include <memory>

class RigidBodyWorker;

class DetectCollisionWorker {
public:
    DetectCollisionWorker() = default;
    ~DetectCollisionWorker() = default;
    
    // Narrow phase collision detection
    void detectCollisions(const std::vector<CollisionPair>& collisionPairs, 
                         std::shared_ptr<RigidBodyWorker> rigidBodyWorker,
                         std::vector<Contact>& contacts,
                         uint32_t maxContacts,
                         uint32_t& contactCount);

private:
    // Collision detection functions
    bool sphereVsSphere(const RigidBody& bodyA, const RigidBody& bodyB, Contact& contact);
    bool sphereVsBox(const RigidBody& sphere, const RigidBody& box, Contact& contact);
    bool boxVsBox(const RigidBody& bodyA, const RigidBody& bodyB, Contact& contact);
    
    // Utility functions
    float combinedRestitution(float restA, float restB) const;
    float combinedFriction(float fricA, float fricB) const;
};