#include "ECSManager.h"
#include "../../../managers/logmanager/Logger.h"
#include <algorithm>
#include <iostream>

namespace gpu_physics {

ECSManager& ECSManager::getInstance() {
    static ECSManager instance;
    return instance;
}

bool ECSManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Reserve memory for entities and components
        entities.reserve(maxParticles);
        particleComponents.reserve(maxParticles);
        particleComponentsWithCPUOffloading.reserve(maxParticles);
        
        std::cout << "GPU ECS Manager initialized with " << maxParticles << " max particles" << std::endl;
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void ECSManager::cleanup() {
    entities.clear();
    particleComponents.clear();
    particleComponentsWithCPUOffloading.clear();
    nextEntityId = 1;
    initialized = false;
}

bool ECSManager::isInitialized() const {
    return initialized;
}

uint32_t ECSManager::createEntity() {
    if (!initialized) {
        return 0;
    }
    
    uint32_t entityId = nextEntityId++;
    entities.push_back(entityId);
    return entityId;
}

bool ECSManager::destroyEntity(uint32_t entityId) {
    if (!initialized) {
        return false;
    }
    
    auto it = std::find(entities.begin(), entities.end(), entityId);
    if (it == entities.end()) {
        return false;
    }
    
    // Remove from all component pools
    particleComponents.erase(entityId);
    particleComponentsWithCPUOffloading.erase(entityId);
    
    // Remove from entity list
    entities.erase(it);
    
    return true;
}

bool ECSManager::isEntityValid(uint32_t entityId) const {
    if (!initialized) {
        return false;
    }
    
    return std::find(entities.begin(), entities.end(), entityId) != entities.end();
}

bool ECSManager::addParticleComponent(uint32_t entityId, const ParticleComponent& component) {
    if (!initialized || !isEntityValid(entityId)) {
        return false;
    }
    
    if (getTotalParticleCount() >= maxParticles) {
        std::cerr << "Cannot add particle component: maximum capacity reached!" << std::endl;
        return false;
    }
    
    particleComponents[entityId] = component;
    return true;
}

bool ECSManager::addParticleComponentWithCPUOffloading(uint32_t entityId, const ParticleComponentWithCPUOffloading& component) {
    if (!initialized || !isEntityValid(entityId)) {
        return false;
    }
    
    if (getTotalParticleCount() >= maxParticles) {
        std::cerr << "Cannot add particle component with CPU offloading: maximum capacity reached!" << std::endl;
        return false;
    }
    
    particleComponentsWithCPUOffloading[entityId] = component;
    return true;
}

bool ECSManager::removeParticleComponent(uint32_t entityId) {
    if (!initialized) {
        return false;
    }
    
    return particleComponents.erase(entityId) > 0;
}

bool ECSManager::removeParticleComponentWithCPUOffloading(uint32_t entityId) {
    if (!initialized) {
        return false;
    }
    
    return particleComponentsWithCPUOffloading.erase(entityId) > 0;
}

ParticleComponent* ECSManager::getParticleComponent(uint32_t entityId) {
    if (!initialized) {
        return nullptr;
    }
    
    auto it = particleComponents.find(entityId);
    return (it != particleComponents.end()) ? &it->second : nullptr;
}

ParticleComponentWithCPUOffloading* ECSManager::getParticleComponentWithCPUOffloading(uint32_t entityId) {
    if (!initialized) {
        return nullptr;
    }
    
    auto it = particleComponentsWithCPUOffloading.find(entityId);
    return (it != particleComponentsWithCPUOffloading.end()) ? &it->second : nullptr;
}

const ParticleComponent* ECSManager::getParticleComponent(uint32_t entityId) const {
    if (!initialized) {
        return nullptr;
    }
    
    auto it = particleComponents.find(entityId);
    return (it != particleComponents.end()) ? &it->second : nullptr;
}

const ParticleComponentWithCPUOffloading* ECSManager::getParticleComponentWithCPUOffloading(uint32_t entityId) const {
    if (!initialized) {
        return nullptr;
    }
    
    auto it = particleComponentsWithCPUOffloading.find(entityId);
    return (it != particleComponentsWithCPUOffloading.end()) ? &it->second : nullptr;
}

bool ECSManager::hasParticleComponent(uint32_t entityId) const {
    if (!initialized) {
        return false;
    }
    
    return particleComponents.find(entityId) != particleComponents.end();
}

bool ECSManager::hasParticleComponentWithCPUOffloading(uint32_t entityId) const {
    if (!initialized) {
        return false;
    }
    
    return particleComponentsWithCPUOffloading.find(entityId) != particleComponentsWithCPUOffloading.end();
}

std::vector<uint32_t> ECSManager::getEntitiesWithParticleComponent() const {
    std::vector<uint32_t> result;
    if (!initialized) {
        return result;
    }
    
    result.reserve(particleComponents.size());
    for (const auto& pair : particleComponents) {
        result.push_back(pair.first);
    }
    return result;
}

std::vector<uint32_t> ECSManager::getEntitiesWithParticleComponentWithCPUOffloading() const {
    std::vector<uint32_t> result;
    if (!initialized) {
        return result;
    }
    
    result.reserve(particleComponentsWithCPUOffloading.size());
    for (const auto& pair : particleComponentsWithCPUOffloading) {
        result.push_back(pair.first);
    }
    return result;
}

std::vector<uint32_t> ECSManager::getAllParticleEntities() const {
    std::vector<uint32_t> result;
    if (!initialized) {
        return result;
    }
    
    auto regularEntities = getEntitiesWithParticleComponent();
    auto cpuOffloadingEntities = getEntitiesWithParticleComponentWithCPUOffloading();
    
    result.reserve(regularEntities.size() + cpuOffloadingEntities.size());
    result.insert(result.end(), regularEntities.begin(), regularEntities.end());
    result.insert(result.end(), cpuOffloadingEntities.begin(), cpuOffloadingEntities.end());
    
    return result;
}

std::vector<ParticleComponent> ECSManager::getParticleComponentData() const {
    std::vector<ParticleComponent> result;
    if (!initialized) {
        return result;
    }
    
    result.reserve(particleComponents.size());
    for (const auto& pair : particleComponents) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<ParticleComponentWithCPUOffloading> ECSManager::getParticleComponentWithCPUOffloadingData() const {
    std::vector<ParticleComponentWithCPUOffloading> result;
    if (!initialized) {
        return result;
    }
    
    result.reserve(particleComponentsWithCPUOffloading.size());
    for (const auto& pair : particleComponentsWithCPUOffloading) {
        result.push_back(pair.second);
    }
    return result;
}

bool ECSManager::setMaxParticles(uint32_t newMaxParticles) {
    if (initialized) {
        // Cannot change after initialization
        return false;
    }
    
    maxParticles = newMaxParticles;
    return true;
}

} // namespace gpu_physics