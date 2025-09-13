#include "CollisionManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../physicsmanager/PhysicsManager.h"
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
        
        // Get buffer manager from PhysicsManager
        auto& physicsManager = PhysicsManager::getInstance();
        auto bufferManager = physicsManager.getBufferManager();
        if (!bufferManager) {
            return false;
        }
        
        // Create collision system
        collisionSystem = std::make_shared<CollisionSystem>(vulkanManager.getContext(), bufferManager);
        if (!collisionSystem->initialize(1024)) { // Default max contacts
            cleanup();
            return false;
        }
        
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