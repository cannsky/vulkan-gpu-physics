#pragma once

#include "../interfaces/CPUPhysicsComponent.h"
#include "../components.h"
#include <memory>

namespace cpu_physics {
namespace concrete {

/**
 * Concrete implementation of CPUPhysicsComponent for TransformComponent
 */
class ConcreteTransformComponent : public interfaces::CPUPhysicsComponent {
public:
    explicit ConcreteTransformComponent(const TransformComponent& transform);
    explicit ConcreteTransformComponent(TransformComponent&& transform);
    ~ConcreteTransformComponent() override = default;

    // CPUPhysicsComponent interface
    ComponentType getType() const override;
    const char* getTypeName() const override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    bool validate() const override;
    void reset() override;
    std::unique_ptr<CPUPhysicsComponent> clone() const override;
    size_t getDataSize() const override;

    // Access to underlying component
    TransformComponent& getTransform() { return transform; }
    const TransformComponent& getTransform() const { return transform; }

private:
    TransformComponent transform;
    bool enabled = true;
};

/**
 * Concrete implementation of CPUPhysicsComponent for PhysicsComponent
 */
class ConcretePhysicsComponent : public interfaces::CPUPhysicsComponent {
public:
    explicit ConcretePhysicsComponent(const PhysicsComponent& physics);
    explicit ConcretePhysicsComponent(PhysicsComponent&& physics);
    ~ConcretePhysicsComponent() override = default;

    // CPUPhysicsComponent interface
    ComponentType getType() const override;
    const char* getTypeName() const override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    bool validate() const override;
    void reset() override;
    std::unique_ptr<CPUPhysicsComponent> clone() const override;
    size_t getDataSize() const override;

    // Access to underlying component
    PhysicsComponent& getPhysics() { return physics; }
    const PhysicsComponent& getPhysics() const { return physics; }

private:
    PhysicsComponent physics;
    bool enabled = true;
};

/**
 * Concrete implementation of CPUPhysicsComponent for BoxColliderComponent
 */
class ConcreteBoxColliderComponent : public interfaces::CPUPhysicsComponent {
public:
    explicit ConcreteBoxColliderComponent(const BoxColliderComponent& collider);
    explicit ConcreteBoxColliderComponent(BoxColliderComponent&& collider);
    ~ConcreteBoxColliderComponent() override = default;

    // CPUPhysicsComponent interface
    ComponentType getType() const override;
    const char* getTypeName() const override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    bool validate() const override;
    void reset() override;
    std::unique_ptr<CPUPhysicsComponent> clone() const override;
    size_t getDataSize() const override;

    // Access to underlying component
    BoxColliderComponent& getCollider() { return collider; }
    const BoxColliderComponent& getCollider() const { return collider; }

private:
    BoxColliderComponent collider;
};

} // namespace concrete
} // namespace cpu_physics