#include "PhysicsManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../particlemanager/ParticleManager.h"
#include "../collisionmanager/CollisionManager.h"
#include "../../physics/components/BufferManager.h"
#include "../../physics/components/ComputePipeline.h"

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
        
        // For now, skip complex component initialization to avoid circular dependencies
        // This can be added back once we resolve all the Vulkan class dependencies
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void PhysicsManager::cleanup() {
    auto& vulkanManager = VulkanManager::getInstance();
    
    if (computeCommandBuffer != VK_NULL_HANDLE && vulkanManager.isInitialized()) {
        // Command buffer will be freed when command pool is destroyed
        computeCommandBuffer = VK_NULL_HANDLE;
    }
    
    if (computePipeline) {
        computePipeline->cleanup();
        computePipeline.reset();
    }
    
    if (bufferManager) {
        bufferManager->cleanup();
        bufferManager.reset();
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

void PhysicsManager::recordComputeCommandBuffer() {
    // Simplified for now - will be implemented once Vulkan dependencies are resolved
}