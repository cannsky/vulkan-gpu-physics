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
        
        // Create buffer manager
        bufferManager = std::make_shared<BufferManager>(vulkanManager.getContext());
        if (!bufferManager->initialize(maxParticles)) {
            cleanup();
            return false;
        }
        
        // Create compute pipeline
        computePipeline = std::make_shared<ComputePipeline>(vulkanManager.getContext());
        if (!computePipeline->initialize()) {
            cleanup();
            return false;
        }
        
        // Initialize subsystems
        auto& particleManager = ParticleManager::getInstance();
        if (!particleManager.initialize()) {
            cleanup();
            return false;
        }
        
        auto& collisionManager = CollisionManager::getInstance();
        if (!collisionManager.initialize()) {
            cleanup();
            return false;
        }
        
        // Create command buffer
        recordComputeCommandBuffer();
        
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
    if (!computePipeline) {
        return;
    }
    
    auto& vulkanManager = VulkanManager::getInstance();
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanManager.getCommandPool()->getCommandPool();
    allocInfo.commandBufferCount = 1;
    
    vkAllocateCommandBuffers(vulkanManager.getLogicalDevice(), &allocInfo, &computeCommandBuffer);
    
    // Record command buffer will be done during physics updates
}