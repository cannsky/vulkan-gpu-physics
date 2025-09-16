#include "GPUPhysicsManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../particlemanager/ParticleManager.h"
#include "../collisionmanager/CollisionManager.h"

GPUPhysicsManager& GPUPhysicsManager::getInstance() {
    static GPUPhysicsManager instance;
    return instance;
}

bool GPUPhysicsManager::initialize() {
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
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void GPUPhysicsManager::cleanup() {
    // Cleanup subsystems
    ParticleManager::getInstance().cleanup();
    CollisionManager::getInstance().cleanup();
    
    initialized = false;
}

bool GPUPhysicsManager::isInitialized() const {
    return initialized;
}

void GPUPhysicsManager::updatePhysics(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    // Update particle system
    auto particleManager = getParticleManager();
    if (particleManager && particleManager->isInitialized()) {
        particleManager->updatePhysics(deltaTime);
    }
    
    // Update collision system for fluid interactions
    auto collisionManager = getCollisionManager();
    if (collisionManager && collisionManager->isInitialized()) {
        // Handle fluid-rigidbody interactions (approximations)
        collisionManager->updateCollisions(deltaTime);
    }
}

void GPUPhysicsManager::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    // Propagate to particle system
    auto particleManager = getParticleManager();
    if (particleManager && particleManager->isInitialized()) {
        particleManager->setGravity(x, y, z);
    }
}

bool GPUPhysicsManager::setMaxParticles(uint32_t newMaxParticles) {
    if (initialized) {
        // Cannot change after initialization
        return false;
    }
    
    maxParticles = newMaxParticles;
    return true;
}

std::shared_ptr<ParticleManager> GPUPhysicsManager::getParticleManager() const {
    return std::shared_ptr<ParticleManager>(&ParticleManager::getInstance(), [](ParticleManager*){});
}

std::shared_ptr<CollisionManager> GPUPhysicsManager::getCollisionManager() const {
    return std::shared_ptr<CollisionManager>(&CollisionManager::getInstance(), [](CollisionManager*){});
}