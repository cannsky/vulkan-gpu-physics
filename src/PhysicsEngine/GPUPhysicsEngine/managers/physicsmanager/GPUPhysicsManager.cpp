#include "GPUPhysicsManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../ecsmanager/ECSManager.h"
#include "../../systems/ParticlePhysicsSystem.h"
#include <iostream>

GPUPhysicsManager& GPUPhysicsManager::getInstance() {
    static GPUPhysicsManager instance;
    return instance;
}

bool GPUPhysicsManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Ensure VulkanManager is initialized (optional for CPU-only builds)
        auto& vulkanManager = VulkanManager::getInstance();
        if (!vulkanManager.isInitialized()) {
            if (!vulkanManager.initialize()) {
                std::cout << "Warning: Vulkan Manager failed to initialize, GPU operations will be limited" << std::endl;
            }
        }
        
        // Initialize ECS Manager
        ecsManager = std::shared_ptr<gpu_physics::ECSManager>(&gpu_physics::ECSManager::getInstance(), [](gpu_physics::ECSManager*){});
        ecsManager->setMaxParticles(maxParticles);
        if (!ecsManager->initialize()) {
            return false;
        }
        
        // Initialize Particle Physics System
        particlePhysicsSystem = std::make_shared<gpu_physics::ParticlePhysicsSystem>();
        particlePhysicsSystem->setECSManager(ecsManager);
        
        // Set VulkanManager if available
        if (vulkanManager.isInitialized()) {
            auto vulkanManagerPtr = std::shared_ptr<VulkanManager>(&vulkanManager, [](VulkanManager*){});
            particlePhysicsSystem->setVulkanManager(vulkanManagerPtr);
        }
        
        if (!particlePhysicsSystem->initialize()) {
            return false;
        }
        
        // Set initial gravity
        particlePhysicsSystem->setGravity(gravity.x, gravity.y, gravity.z);
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void GPUPhysicsManager::cleanup() {
    // Cleanup subsystems
    if (particlePhysicsSystem) {
        particlePhysicsSystem->cleanup();
        particlePhysicsSystem.reset();
    }
    
    if (ecsManager) {
        ecsManager->cleanup();
        ecsManager.reset();
    }
    
    initialized = false;
}

bool GPUPhysicsManager::isInitialized() const {
    return initialized;
}

void GPUPhysicsManager::updatePhysics(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    // Update particle physics system
    if (particlePhysicsSystem && particlePhysicsSystem->isInitialized()) {
        particlePhysicsSystem->updatePhysics(deltaTime);
    }
}

void GPUPhysicsManager::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    // Propagate to particle physics system
    if (particlePhysicsSystem && particlePhysicsSystem->isInitialized()) {
        particlePhysicsSystem->setGravity(x, y, z);
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

std::shared_ptr<gpu_physics::ECSManager> GPUPhysicsManager::getECSManager() const {
    return ecsManager;
}

std::shared_ptr<gpu_physics::ParticlePhysicsSystem> GPUPhysicsManager::getParticlePhysicsSystem() const {
    return particlePhysicsSystem;
}