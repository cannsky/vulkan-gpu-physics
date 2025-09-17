#pragma once

#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <cstdint>

namespace cpu_physics {

// Forward declarations
struct TransformComponent;
struct PhysicsComponent;
struct BoxColliderComponent;

/**
 * ECS Manager - Simplified implementation for CPU physics components
 * 
 * This implementation uses separate typed storage for each component type
 * instead of complex type-erasure for better performance and simplicity.
 */
class ECSManager {
public:
    ECSManager();
    ~ECSManager() = default;
    
    // Entity management
    uint32_t createEntity();
    bool destroyEntity(uint32_t entityId);
    bool isEntityValid(uint32_t entityId) const;
    
    // Component management (explicit types for physics)
    bool addComponent(uint32_t entityId, const TransformComponent& component);
    bool addComponent(uint32_t entityId, const PhysicsComponent& component);
    bool addComponent(uint32_t entityId, const BoxColliderComponent& component);
    
    bool removeComponent(uint32_t entityId, std::type_index componentType);
    
    // Typed component access
    TransformComponent* getTransformComponent(uint32_t entityId);
    PhysicsComponent* getPhysicsComponent(uint32_t entityId);
    BoxColliderComponent* getBoxColliderComponent(uint32_t entityId);
    
    const TransformComponent* getTransformComponent(uint32_t entityId) const;
    const PhysicsComponent* getPhysicsComponent(uint32_t entityId) const;
    const BoxColliderComponent* getBoxColliderComponent(uint32_t entityId) const;
    
    bool hasTransformComponent(uint32_t entityId) const;
    bool hasPhysicsComponent(uint32_t entityId) const;
    bool hasBoxColliderComponent(uint32_t entityId) const;
    
    // Entity queries
    std::vector<uint32_t> getEntitiesWithTransformComponent() const;
    std::vector<uint32_t> getEntitiesWithPhysicsComponent() const;
    std::vector<uint32_t> getEntitiesWithBoxColliderComponent() const;
    
    // Template helpers for generic access
    template<typename T>
    T* getComponent(uint32_t entityId);
    
    template<typename T>
    const T* getComponent(uint32_t entityId) const;
    
    template<typename T>
    bool hasComponent(uint32_t entityId) const;
    
    template<typename T>
    bool addComponent(uint32_t entityId, const T& component);
    
    template<typename T>
    bool removeComponent(uint32_t entityId);
    
    template<typename T>
    std::vector<uint32_t> getEntitiesWithComponent() const;
    
    // Multi-component query methods
    template<typename T1, typename T2, typename T3>
    std::vector<uint32_t> getEntitiesWith() const;
    
    template<typename T1, typename T2, typename T3>
    size_t getEntityCountWith() const;
    
    // Statistics
    size_t getEntityCount() const { return entities.size(); }
    size_t getComponentTypeCount() const { return 3; } // Transform, Physics, BoxCollider

private:
    // Entity storage
    std::vector<uint32_t> entities;
    uint32_t nextEntityId = 1;
    
    // Component storage (typed)
    std::unordered_map<uint32_t, TransformComponent> transformComponents;
    std::unordered_map<uint32_t, PhysicsComponent> physicsComponents;
    std::unordered_map<uint32_t, BoxColliderComponent> boxColliderComponents;
};

// Template specializations
template<>
TransformComponent* ECSManager::getComponent<TransformComponent>(uint32_t entityId);

template<>
PhysicsComponent* ECSManager::getComponent<PhysicsComponent>(uint32_t entityId);

template<>
BoxColliderComponent* ECSManager::getComponent<BoxColliderComponent>(uint32_t entityId);

template<>
const TransformComponent* ECSManager::getComponent<TransformComponent>(uint32_t entityId) const;

template<>
const PhysicsComponent* ECSManager::getComponent<PhysicsComponent>(uint32_t entityId) const;

template<>
const BoxColliderComponent* ECSManager::getComponent<BoxColliderComponent>(uint32_t entityId) const;

template<>
bool ECSManager::hasComponent<TransformComponent>(uint32_t entityId) const;

template<>
bool ECSManager::hasComponent<PhysicsComponent>(uint32_t entityId) const;

template<>
bool ECSManager::hasComponent<BoxColliderComponent>(uint32_t entityId) const;

template<>
std::vector<uint32_t> ECSManager::getEntitiesWithComponent<TransformComponent>() const;

template<>
std::vector<uint32_t> ECSManager::getEntitiesWithComponent<PhysicsComponent>() const;

template<>
std::vector<uint32_t> ECSManager::getEntitiesWithComponent<BoxColliderComponent>() const;

// Template specializations for addComponent
template<>
bool ECSManager::addComponent<TransformComponent>(uint32_t entityId, const TransformComponent& component);

template<>
bool ECSManager::addComponent<PhysicsComponent>(uint32_t entityId, const PhysicsComponent& component);

template<>
bool ECSManager::addComponent<BoxColliderComponent>(uint32_t entityId, const BoxColliderComponent& component);

// Template specializations for removeComponent
template<>
bool ECSManager::removeComponent<TransformComponent>(uint32_t entityId);

template<>
bool ECSManager::removeComponent<PhysicsComponent>(uint32_t entityId);

template<>
bool ECSManager::removeComponent<BoxColliderComponent>(uint32_t entityId);

// Template specializations for multi-component queries
template<>
std::vector<uint32_t> ECSManager::getEntitiesWith<TransformComponent, PhysicsComponent, BoxColliderComponent>() const;

template<>
size_t ECSManager::getEntityCountWith<TransformComponent, PhysicsComponent, BoxColliderComponent>() const;

} // namespace cpu_physics