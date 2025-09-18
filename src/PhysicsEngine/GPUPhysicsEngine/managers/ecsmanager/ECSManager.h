#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>
#include "../../../managers/BaseManager.h"
#include "../../components/ParticleComponent.h"

namespace gpu_physics {

/**
 * ECS Manager for GPU particle physics system.
 * Manages entities and particle components for efficient GPU data transfer.
 * 
 * This system is designed to be similar to the CPU physics ECS but optimized 
 * for GPU particle data handling with static CPU offloading configuration.
 */
class ECSManager : public BaseManager {
public:
    static ECSManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Entity management
    uint32_t createEntity();
    bool destroyEntity(uint32_t entityId);
    bool isEntityValid(uint32_t entityId) const;
    
    // Particle component management
    bool addParticleComponent(uint32_t entityId, const ParticleComponent& component);
    bool addParticleComponentWithCPUOffloading(uint32_t entityId, const ParticleComponentWithCPUOffloading& component);
    bool removeParticleComponent(uint32_t entityId);
    bool removeParticleComponentWithCPUOffloading(uint32_t entityId);
    
    // Component access
    ParticleComponent* getParticleComponent(uint32_t entityId);
    ParticleComponentWithCPUOffloading* getParticleComponentWithCPUOffloading(uint32_t entityId);
    const ParticleComponent* getParticleComponent(uint32_t entityId) const;
    const ParticleComponentWithCPUOffloading* getParticleComponentWithCPUOffloading(uint32_t entityId) const;
    
    bool hasParticleComponent(uint32_t entityId) const;
    bool hasParticleComponentWithCPUOffloading(uint32_t entityId) const;
    
    // Entity queries
    std::vector<uint32_t> getEntitiesWithParticleComponent() const;
    std::vector<uint32_t> getEntitiesWithParticleComponentWithCPUOffloading() const;
    std::vector<uint32_t> getAllParticleEntities() const;
    
    // Component data access for GPU operations
    std::vector<ParticleComponent> getParticleComponentData() const;
    std::vector<ParticleComponentWithCPUOffloading> getParticleComponentWithCPUOffloadingData() const;
    
    // Statistics
    size_t getEntityCount() const { return entities.size(); }
    size_t getParticleComponentCount() const { return particleComponents.size(); }
    size_t getParticleComponentWithCPUOffloadingCount() const { return particleComponentsWithCPUOffloading.size(); }
    size_t getTotalParticleCount() const { return particleComponents.size() + particleComponentsWithCPUOffloading.size(); }
    
    // Configuration
    uint32_t getMaxParticles() const { return maxParticles; }
    bool setMaxParticles(uint32_t maxParticles);

private:
    ECSManager() = default;
    ~ECSManager() = default;
    
    bool initialized = false;
    uint32_t maxParticles = 1024;
    
    // Entity storage
    std::vector<uint32_t> entities;
    uint32_t nextEntityId = 1;
    
    // Component storage
    std::unordered_map<uint32_t, ParticleComponent> particleComponents;
    std::unordered_map<uint32_t, ParticleComponentWithCPUOffloading> particleComponentsWithCPUOffloading;
};

} // namespace gpu_physics