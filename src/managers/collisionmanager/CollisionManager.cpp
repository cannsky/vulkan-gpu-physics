#include "CollisionManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../../collision/CollisionSystem.h"

CollisionManager& CollisionManager::getInstance() {
    static CollisionManager instance;
    return instance;
}

bool CollisionManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Ensure VulkanManager is initialized
        auto& vulkanManager = VulkanManager::getInstance();
        if (!vulkanManager.isInitialized()) {
            return false;
        }
        
        // For now, skip complex initialization to avoid circular dependencies
        // This will be properly implemented once the dependency chain is resolved
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void CollisionManager::cleanup() {
    if (collisionSystem) {
        collisionSystem->cleanup();
        collisionSystem.reset();
    }
    
    initialized = false;
}

bool CollisionManager::isInitialized() const {
    return initialized;
}

void CollisionManager::updateCollisions(float deltaTime) {
    if (!collisionSystem) {
        return;
    }
    
    detectCollisions();
    resolveContacts(deltaTime);
}

void CollisionManager::detectCollisions() {
    if (!collisionSystem) {
        return;
    }
    
    // Note: This would need a rigid body system reference
    // For now, we'll just update the broad phase with empty data
    std::vector<RigidBody> emptyRigidBodies;
    collisionSystem->updateBroadPhase(emptyRigidBodies);
}

void CollisionManager::resolveContacts(float deltaTime) {
    if (!collisionSystem) {
        return;
    }
    
    collisionSystem->resolveContacts(deltaTime);
}

uint32_t CollisionManager::getContactCount() const {
    if (!collisionSystem) {
        return 0;
    }
    return collisionSystem->getContactCount();
}

uint32_t CollisionManager::getCollisionPairCount() const {
    if (!collisionSystem) {
        return 0;
    }
    return collisionSystem->getCollisionPairCount();
}

void CollisionManager::uploadContactsToGPU() {
    if (!collisionSystem) {
        return;
    }
    collisionSystem->uploadContactsToGPU();
}

void CollisionManager::downloadContactsFromGPU() {
    if (!collisionSystem) {
        return;
    }
    collisionSystem->downloadContactsFromGPU();
}