#include "BaseCPUPhysicsSystem.h"
#include "../../managers/logmanager/Logger.h"
#include <chrono>

namespace cpu_physics {

BaseCPUPhysicsSystem::BaseCPUPhysicsSystem(std::shared_ptr<ECSManager> ecsManager)
    : ecsManager(ecsManager) {
    if (!ecsManager) {
        LOG_ERROR(LogCategory::PHYSICS, "BaseCPUPhysicsSystem: ECSManager cannot be null");
    }
}

bool BaseCPUPhysicsSystem::initialize() {
    if (initialized) {
        LOG_WARN(LogCategory::PHYSICS, "BaseCPUPhysicsSystem: Already initialized");
        return true;
    }

    if (!ecsManager) {
        LOG_ERROR(LogCategory::PHYSICS, "BaseCPUPhysicsSystem: Cannot initialize without ECSManager");
        return false;
    }

    initialized = true;
    LOG_DEBUG(LogCategory::PHYSICS, "BaseCPUPhysicsSystem: Initialized successfully");
    return true;
}

void BaseCPUPhysicsSystem::cleanup() {
    if (!initialized) {
        return;
    }

    enabled = false;
    initialized = false;
    entityFilter = nullptr;
    lastUpdateTime = 0.0f;
    lastEntityCount = 0;

    LOG_DEBUG(LogCategory::PHYSICS, "BaseCPUPhysicsSystem: Cleaned up successfully");
}

void BaseCPUPhysicsSystem::update(float deltaTime) {
    if (!initialized || !enabled) {
        return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Call the derived class implementation
    updateInternal(deltaTime);

    auto endTime = std::chrono::high_resolution_clock::now();
    float updateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    updateStatistics(updateTime, lastEntityCount);
}

void BaseCPUPhysicsSystem::update(const std::vector<interfaces::CPUPhysicsEntity*>& entities, float deltaTime) {
    if (!initialized || !enabled) {
        return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Process entities using the interface
    preUpdate(deltaTime);

    size_t processedCount = 0;
    for (auto* entity : entities) {
        if (entity && canProcessEntity(entity)) {
            if (!entityFilter || entityFilter(entity)) {
                processEntity(entity, deltaTime);
                processedCount++;
            }
        }
    }

    postUpdate(deltaTime);

    auto endTime = std::chrono::high_resolution_clock::now();
    float updateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    updateStatistics(updateTime, processedCount);
}

bool BaseCPUPhysicsSystem::canProcessEntity(const interfaces::CPUPhysicsEntity* entity) const {
    if (!entity || !entity->isActive()) {
        return false;
    }

    // Check if entity has all required components
    auto requiredComponents = getRequiredComponents();
    for (auto componentType : requiredComponents) {
        if (!entity->hasComponent(componentType)) {
            return false;
        }
    }

    return true;
}

bool BaseCPUPhysicsSystem::validate() const {
    if (!ecsManager) {
        return false;
    }

    return initialized;
}

void BaseCPUPhysicsSystem::reset() {
    cleanup();
}

void BaseCPUPhysicsSystem::setEntityFilter(std::function<bool(const interfaces::CPUPhysicsEntity*)> filter) {
    entityFilter = filter;
}

void BaseCPUPhysicsSystem::clearEntityFilter() {
    entityFilter = nullptr;
}

std::string BaseCPUPhysicsSystem::getDebugInfo() const {
    std::string info = "BaseCPUPhysicsSystem Debug Info:\n";
    info += "  Initialized: " + std::string(initialized ? "true" : "false") + "\n";
    info += "  Enabled: " + std::string(enabled ? "true" : "false") + "\n";
    info += "  Last Update Time: " + std::to_string(lastUpdateTime) + " ms\n";
    info += "  Last Entity Count: " + std::to_string(lastEntityCount) + "\n";
    info += "  Has Entity Filter: " + std::string(entityFilter ? "true" : "false") + "\n";
    info += "  ECS Manager: " + std::string(ecsManager ? "valid" : "null") + "\n";
    return info;
}

void BaseCPUPhysicsSystem::updateStatistics(float updateTime, size_t entityCount) {
    lastUpdateTime = updateTime;
    lastEntityCount = entityCount;
}

std::vector<uint32_t> BaseCPUPhysicsSystem::getEntitiesWithRequiredComponents() const {
    if (!ecsManager) {
        return {};
    }

    auto requiredComponents = getRequiredComponents();
    if (requiredComponents.empty()) {
        return {};
    }

    // Start with entities that have the first required component
    // For now, we'll use Transform as the primary component to get entities
    auto entities = ecsManager->getEntitiesWithComponent<TransformComponent>();
    
    // Filter entities that have all required components
    std::vector<uint32_t> validEntities;
    for (uint32_t entityId : entities) {
        if (entityHasRequiredComponents(entityId)) {
            validEntities.push_back(entityId);
        }
    }
    
    return validEntities;
}

bool BaseCPUPhysicsSystem::entityHasRequiredComponents(uint32_t entityId) const {
    if (!ecsManager) {
        return false;
    }

    auto requiredComponents = getRequiredComponents();
    for (auto componentType : requiredComponents) {
        switch (componentType) {
            case interfaces::CPUPhysicsComponent::ComponentType::TRANSFORM:
                if (!ecsManager->hasComponent<TransformComponent>(entityId)) {
                    return false;
                }
                break;
            case interfaces::CPUPhysicsComponent::ComponentType::PHYSICS:
                if (!ecsManager->hasComponent<PhysicsComponent>(entityId)) {
                    return false;
                }
                break;
            case interfaces::CPUPhysicsComponent::ComponentType::BOX_COLLIDER:
                if (!ecsManager->hasComponent<BoxColliderComponent>(entityId)) {
                    return false;
                }
                break;
            default:
                // Unknown component type, assume entity doesn't have it
                return false;
        }
    }
    
    return true;
}

} // namespace cpu_physics