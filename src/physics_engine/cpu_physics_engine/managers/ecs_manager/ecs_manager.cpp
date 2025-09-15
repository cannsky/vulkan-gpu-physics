#include "ecs_manager.h"
#include "../../components.h"
#include "../../../managers/logmanager/Logger.h"
#include <algorithm>

namespace cpu_physics {

ECSManager::ECSManager() {
    LOG_INFO(LogCategory::PHYSICS, "Creating ECS Manager for physics components");
}

uint32_t ECSManager::createEntity() {
    uint32_t entityId = nextEntityId++;
    entities.push_back(entityId);
    
    LOG_DEBUG(LogCategory::PHYSICS, "Created entity " + std::to_string(entityId));
    return entityId;
}

bool ECSManager::destroyEntity(uint32_t entityId) {
    auto it = std::find(entities.begin(), entities.end(), entityId);
    if (it == entities.end()) {
        return false;
    }
    
    // Remove from all component pools
    transformComponents.erase(entityId);
    physicsComponents.erase(entityId);
    boxColliderComponents.erase(entityId);
    
    // Remove from entity list
    entities.erase(it);
    
    LOG_DEBUG(LogCategory::PHYSICS, "Destroyed entity " + std::to_string(entityId));
    return true;
}

bool ECSManager::isEntityValid(uint32_t entityId) const {
    return std::find(entities.begin(), entities.end(), entityId) != entities.end();
}

bool ECSManager::addComponent(uint32_t entityId, const TransformComponent& component) {
    if (!isEntityValid(entityId)) {
        return false;
    }
    transformComponents[entityId] = component;
    return true;
}

bool ECSManager::addComponent(uint32_t entityId, const PhysicsComponent& component) {
    if (!isEntityValid(entityId)) {
        return false;
    }
    physicsComponents[entityId] = component;
    return true;
}

bool ECSManager::addComponent(uint32_t entityId, const BoxColliderComponent& component) {
    if (!isEntityValid(entityId)) {
        return false;
    }
    boxColliderComponents[entityId] = component;
    return true;
}

bool ECSManager::removeComponent(uint32_t entityId, std::type_index componentType) {
    if (componentType == std::type_index(typeid(TransformComponent))) {
        return transformComponents.erase(entityId) > 0;
    } else if (componentType == std::type_index(typeid(PhysicsComponent))) {
        return physicsComponents.erase(entityId) > 0;
    } else if (componentType == std::type_index(typeid(BoxColliderComponent))) {
        return boxColliderComponents.erase(entityId) > 0;
    }
    return false;
}

TransformComponent* ECSManager::getTransformComponent(uint32_t entityId) {
    auto it = transformComponents.find(entityId);
    return (it != transformComponents.end()) ? &it->second : nullptr;
}

PhysicsComponent* ECSManager::getPhysicsComponent(uint32_t entityId) {
    auto it = physicsComponents.find(entityId);
    return (it != physicsComponents.end()) ? &it->second : nullptr;
}

BoxColliderComponent* ECSManager::getBoxColliderComponent(uint32_t entityId) {
    auto it = boxColliderComponents.find(entityId);
    return (it != boxColliderComponents.end()) ? &it->second : nullptr;
}

const TransformComponent* ECSManager::getTransformComponent(uint32_t entityId) const {
    auto it = transformComponents.find(entityId);
    return (it != transformComponents.end()) ? &it->second : nullptr;
}

const PhysicsComponent* ECSManager::getPhysicsComponent(uint32_t entityId) const {
    auto it = physicsComponents.find(entityId);
    return (it != physicsComponents.end()) ? &it->second : nullptr;
}

const BoxColliderComponent* ECSManager::getBoxColliderComponent(uint32_t entityId) const {
    auto it = boxColliderComponents.find(entityId);
    return (it != boxColliderComponents.end()) ? &it->second : nullptr;
}

bool ECSManager::hasTransformComponent(uint32_t entityId) const {
    return transformComponents.find(entityId) != transformComponents.end();
}

bool ECSManager::hasPhysicsComponent(uint32_t entityId) const {
    return physicsComponents.find(entityId) != physicsComponents.end();
}

bool ECSManager::hasBoxColliderComponent(uint32_t entityId) const {
    return boxColliderComponents.find(entityId) != boxColliderComponents.end();
}

std::vector<uint32_t> ECSManager::getEntitiesWithTransformComponent() const {
    std::vector<uint32_t> result;
    for (const auto& [entityId, component] : transformComponents) {
        result.push_back(entityId);
    }
    return result;
}

std::vector<uint32_t> ECSManager::getEntitiesWithPhysicsComponent() const {
    std::vector<uint32_t> result;
    for (const auto& [entityId, component] : physicsComponents) {
        result.push_back(entityId);
    }
    return result;
}

std::vector<uint32_t> ECSManager::getEntitiesWithBoxColliderComponent() const {
    std::vector<uint32_t> result;
    for (const auto& [entityId, component] : boxColliderComponents) {
        result.push_back(entityId);
    }
    return result;
}

// Template specializations
template<>
TransformComponent* ECSManager::getComponent<TransformComponent>(uint32_t entityId) {
    return getTransformComponent(entityId);
}

template<>
PhysicsComponent* ECSManager::getComponent<PhysicsComponent>(uint32_t entityId) {
    return getPhysicsComponent(entityId);
}

template<>
BoxColliderComponent* ECSManager::getComponent<BoxColliderComponent>(uint32_t entityId) {
    return getBoxColliderComponent(entityId);
}

template<>
const TransformComponent* ECSManager::getComponent<TransformComponent>(uint32_t entityId) const {
    return getTransformComponent(entityId);
}

template<>
const PhysicsComponent* ECSManager::getComponent<PhysicsComponent>(uint32_t entityId) const {
    return getPhysicsComponent(entityId);
}

template<>
const BoxColliderComponent* ECSManager::getComponent<BoxColliderComponent>(uint32_t entityId) const {
    return getBoxColliderComponent(entityId);
}

template<>
bool ECSManager::hasComponent<TransformComponent>(uint32_t entityId) const {
    return hasTransformComponent(entityId);
}

template<>
bool ECSManager::hasComponent<PhysicsComponent>(uint32_t entityId) const {
    return hasPhysicsComponent(entityId);
}

template<>
bool ECSManager::hasComponent<BoxColliderComponent>(uint32_t entityId) const {
    return hasBoxColliderComponent(entityId);
}

template<>
std::vector<uint32_t> ECSManager::getEntitiesWithComponent<TransformComponent>() const {
    return getEntitiesWithTransformComponent();
}

template<>
std::vector<uint32_t> ECSManager::getEntitiesWithComponent<PhysicsComponent>() const {
    return getEntitiesWithPhysicsComponent();
}

template<>
std::vector<uint32_t> ECSManager::getEntitiesWithComponent<BoxColliderComponent>() const {
    return getEntitiesWithBoxColliderComponent();
}

} // namespace cpu_physics