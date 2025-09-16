#pragma once

#include "../interfaces/CPUPhysicsEntity.h"
#include "../interfaces/CPUPhysicsComponent.h"
#include <unordered_map>
#include <memory>
#include <vector>

namespace cpu_physics {
namespace concrete {

/**
 * Concrete implementation of CPUPhysicsEntity
 * 
 * This class provides a complete implementation of the CPUPhysicsEntity interface
 * using a component storage system based on unordered_map.
 */
class ConcreteEntity : public interfaces::CPUPhysicsEntity {
public:
    explicit ConcreteEntity(uint32_t id);
    ~ConcreteEntity() override = default;

    // CPUPhysicsEntity interface implementation
    uint32_t getId() const override;
    bool isActive() const override;
    void setActive(bool active) override;

    bool addComponent(std::unique_ptr<interfaces::CPUPhysicsComponent> component) override;
    bool removeComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) override;
    
    interfaces::CPUPhysicsComponent* getComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) override;
    const interfaces::CPUPhysicsComponent* getComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) const override;
    
    bool hasComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) const override;
    
    std::vector<interfaces::CPUPhysicsComponent*> getAllComponents() override;
    std::vector<const interfaces::CPUPhysicsComponent*> getAllComponents() const override;
    
    size_t getComponentCount() const override;
    bool validate() const override;
    void reset() override;
    std::unique_ptr<interfaces::CPUPhysicsEntity> clone() const override;

    uint32_t getPhysicsLayer() const override;
    void setPhysicsLayer(uint32_t layer) override;

    void* getUserData() const override;
    void setUserData(void* userData) override;

private:
    uint32_t entityId;
    bool active = true;
    uint32_t physicsLayer = 0;
    void* userData = nullptr;
    
    // Component storage using component type as key
    std::unordered_map<interfaces::CPUPhysicsComponent::ComponentType, 
                       std::unique_ptr<interfaces::CPUPhysicsComponent>> components;
    
    // Helper to convert ComponentType to size_t for hash map
    size_t componentTypeToIndex(interfaces::CPUPhysicsComponent::ComponentType type) const;
};

/**
 * Factory for creating concrete entities with common component configurations
 */
class ConcreteEntityFactory {
public:
    ConcreteEntityFactory() = default;
    ~ConcreteEntityFactory() = default;

    /**
     * Create a basic entity with just an ID
     */
    static std::unique_ptr<ConcreteEntity> createBasicEntity(uint32_t id);

    /**
     * Create a rigidbody entity with Transform, Physics, and BoxCollider components
     */
    static std::unique_ptr<ConcreteEntity> createRigidBodyEntity(
        uint32_t id,
        float x = 0.0f, float y = 0.0f, float z = 0.0f,
        float width = 1.0f, float height = 1.0f, float depth = 1.0f,
        float mass = 1.0f,
        uint32_t layer = 0
    );

    /**
     * Create a static entity (no physics but has transform and collider)
     */
    static std::unique_ptr<ConcreteEntity> createStaticEntity(
        uint32_t id,
        float x = 0.0f, float y = 0.0f, float z = 0.0f,
        float width = 1.0f, float height = 1.0f, float depth = 1.0f,
        uint32_t layer = 0
    );

private:
    static uint32_t nextEntityId;
};

} // namespace concrete
} // namespace cpu_physics