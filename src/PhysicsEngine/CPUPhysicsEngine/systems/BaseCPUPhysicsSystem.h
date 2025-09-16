#pragma once

#include "../interfaces/CPUPhysicsSystem.h"
#include "../managers/ECSManager/ECSManager.h"
#include <memory>
#include <string>
#include <chrono>

namespace cpu_physics {

/**
 * Base implementation of CPUPhysicsSystem interface
 * 
 * This class provides common functionality for ECS-based physics systems
 * and serves as a bridge between the interface and concrete implementations.
 * It can be used as a base class for systems that work with the ECS architecture.
 */
class BaseCPUPhysicsSystem : public interfaces::CPUPhysicsSystem {
public:
    explicit BaseCPUPhysicsSystem(std::shared_ptr<ECSManager> ecsManager);
    virtual ~BaseCPUPhysicsSystem() = default;

    // CPUPhysicsSystem interface implementation
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override { return initialized; }

    void update(float deltaTime) override;
    void update(const std::vector<interfaces::CPUPhysicsEntity*>& entities, float deltaTime) override;

    bool canProcessEntity(const interfaces::CPUPhysicsEntity* entity) const override;

    void setEnabled(bool enabled) override { this->enabled = enabled; }
    bool isEnabled() const override { return enabled; }

    bool validate() const override;
    void reset() override;

    float getLastUpdateTime() const override { return lastUpdateTime; }
    size_t getLastEntityCount() const override { return lastEntityCount; }

    void setEntityFilter(std::function<bool(const interfaces::CPUPhysicsEntity*)> filter) override;
    void clearEntityFilter() override;

    std::string getDebugInfo() const override;

protected:
    // ECS access for derived classes
    std::shared_ptr<ECSManager> getECSManager() const { return ecsManager; }

    // Statistics tracking
    void updateStatistics(float updateTime, size_t entityCount);

    // Helper methods for ECS entity processing
    std::vector<uint32_t> getEntitiesWithRequiredComponents() const;
    bool entityHasRequiredComponents(uint32_t entityId) const;

    // Called by update() to do the actual work - derived classes implement this
    virtual void updateInternal(float deltaTime) = 0;

private:
    std::shared_ptr<ECSManager> ecsManager;
    bool initialized = false;
    bool enabled = true;
    
    // Performance tracking
    float lastUpdateTime = 0.0f;
    size_t lastEntityCount = 0;
    
    // Entity filtering
    std::function<bool(const interfaces::CPUPhysicsEntity*)> entityFilter;
};

} // namespace cpu_physics