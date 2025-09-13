#include "PhysicsManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../particlemanager/ParticleManager.h"
#include "../collisionmanager/CollisionManager.h"
#include "workers/PhysicsLayerWorker.h"
#include "workers/RigidBodyWorker.h"

PhysicsManager& PhysicsManager::getInstance() {
    static PhysicsManager instance;
    return instance;
}

bool PhysicsManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Ensure VulkanManager is initialized
        auto& vulkanManager = VulkanManager::getInstance();
        if (!vulkanManager.isInitialized()) {
            if (!vulkanManager.initialize()) {
                return false;
            }
        }
        
        // Initialize physics layer worker
        layerWorker = std::make_shared<PhysicsLayerWorker>();
        if (!layerWorker->initialize()) {
            return false;
        }
        
        // Initialize rigidbody worker
        rigidBodyWorker = std::make_shared<RigidBodyWorker>();
        if (!rigidBodyWorker->initialize()) {
            return false;
        }
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void PhysicsManager::cleanup() {
    if (rigidBodyWorker) {
        rigidBodyWorker->cleanup();
        rigidBodyWorker.reset();
    }
    
    if (layerWorker) {
        layerWorker->cleanup();
        layerWorker.reset();
    }
    
    // Cleanup subsystems
    ParticleManager::getInstance().cleanup();
    CollisionManager::getInstance().cleanup();
    
    initialized = false;
}

bool PhysicsManager::isInitialized() const {
    return initialized;
}

void PhysicsManager::updatePhysics(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    // Update rigidbody system
    if (rigidBodyWorker) {
        rigidBodyWorker->updatePhysics(deltaTime);
    }
    
    // Update particle system
    auto particleManager = getParticleManager();
    if (particleManager && particleManager->isInitialized()) {
        particleManager->updatePhysics(deltaTime);
    }
    
    // Update collision system
    auto collisionManager = getCollisionManager();
    if (collisionManager && collisionManager->isInitialized()) {
        collisionManager->updateCollisions(deltaTime);
    }
}

void PhysicsManager::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    // Propagate to subsystems
    if (rigidBodyWorker) {
        rigidBodyWorker->setGravity(x, y, z);
    }
    
    auto particleManager = getParticleManager();
    if (particleManager && particleManager->isInitialized()) {
        particleManager->setGravity(x, y, z);
    }
}

bool PhysicsManager::setMaxParticles(uint32_t newMaxParticles) {
    if (initialized) {
        // Cannot change after initialization
        return false;
    }
    
    maxParticles = newMaxParticles;
    return true;
}

std::shared_ptr<ParticleManager> PhysicsManager::getParticleManager() const {
    return std::shared_ptr<ParticleManager>(&ParticleManager::getInstance(), [](ParticleManager*){});
}

std::shared_ptr<CollisionManager> PhysicsManager::getCollisionManager() const {
    return std::shared_ptr<CollisionManager>(&CollisionManager::getInstance(), [](CollisionManager*){});
}