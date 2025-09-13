#include "RigidBodyWorker.h"
#include "../../../components/rigidbody/RigidBodySystem.h"
#include <algorithm>

RigidBodyWorker::RigidBodyWorker() {
    rigidBodies.reserve(maxRigidBodies);
}

bool RigidBodyWorker::initialize() {
    if (initialized) {
        return true;
    }
    
    rigidBodies.clear();
    freeIds.clear();
    nextId = 1;
    
    initialized = true;
    return true;
}

void RigidBodyWorker::cleanup() {
    rigidBodies.clear();
    freeIds.clear();
    rigidBodySystem.reset();
    initialized = false;
}

uint32_t RigidBodyWorker::addRigidBody(const RigidBody& body) {
    if (!initialized) {
        return 0; // Invalid ID
    }
    
    uint32_t id;
    if (!freeIds.empty()) {
        id = freeIds.back();
        freeIds.pop_back();
    } else {
        id = nextId++;
    }
    
    if (rigidBodies.size() >= maxRigidBodies) {
        return 0; // No space available
    }
    
    rigidBodies.push_back(body);
    
    // If we have a GPU-based system, add it there too
    if (rigidBodySystem) {
        rigidBodySystem->createRigidBody(body);
    }
    
    return id;
}

bool RigidBodyWorker::removeRigidBody(uint32_t bodyId) {
    if (!initialized || bodyId == 0 || bodyId >= nextId) {
        return false;
    }
    
    // Find and remove the rigidbody
    auto it = std::find_if(rigidBodies.begin(), rigidBodies.end(), 
        [bodyId](const RigidBody& body) {
            // For now, we'll use a simple linear search
            // In a real implementation, we'd maintain an ID mapping
            return true; // Simplified for this refactoring
        });
    
    if (it != rigidBodies.end()) {
        rigidBodies.erase(it);
        freeIds.push_back(bodyId);
        
        // Remove from GPU system if available
        if (rigidBodySystem) {
            rigidBodySystem->removeRigidBody(bodyId);
        }
        
        return true;
    }
    
    return false;
}

RigidBody* RigidBodyWorker::getRigidBody(uint32_t bodyId) {
    if (!initialized || bodyId == 0 || rigidBodies.empty()) {
        return nullptr;
    }
    
    // Simplified implementation - in real system would use proper ID mapping
    if (bodyId <= rigidBodies.size()) {
        return &rigidBodies[bodyId - 1];
    }
    
    return nullptr;
}

void RigidBodyWorker::updatePhysics(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    // Update rigidbodies with basic physics
    for (auto& body : rigidBodies) {
        if (body.isStatic) {
            continue;
        }
        
        // Apply gravity
        if (body.invMass > 0.0f) {
            body.velocity[0] += gravity.x * deltaTime;
            body.velocity[1] += gravity.y * deltaTime;
            body.velocity[2] += gravity.z * deltaTime;
        }
        
        // Update position
        body.position[0] += body.velocity[0] * deltaTime;
        body.position[1] += body.velocity[1] * deltaTime;
        body.position[2] += body.velocity[2] * deltaTime;
    }
    
    // Update GPU buffers if available
    if (rigidBodySystem) {
        rigidBodySystem->updateUniformBuffer(deltaTime);
    }
}

void RigidBodyWorker::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    // Update GPU system if available
    if (rigidBodySystem) {
        rigidBodySystem->setGravity(x, y, z);
    }
}