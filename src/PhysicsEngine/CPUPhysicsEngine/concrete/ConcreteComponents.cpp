#include "ConcreteComponents.h"
#include <cstring>

namespace cpu_physics {
namespace concrete {

// ConcreteTransformComponent implementation
ConcreteTransformComponent::ConcreteTransformComponent(const TransformComponent& transform)
    : transform(transform) {
}

ConcreteTransformComponent::ConcreteTransformComponent(TransformComponent&& transform)
    : transform(std::move(transform)) {
}

interfaces::CPUPhysicsComponent::ComponentType ConcreteTransformComponent::getType() const {
    return ComponentType::TRANSFORM;
}

const char* ConcreteTransformComponent::getTypeName() const {
    return "ConcreteTransformComponent";
}

bool ConcreteTransformComponent::isEnabled() const {
    return enabled;
}

void ConcreteTransformComponent::setEnabled(bool enabled) {
    this->enabled = enabled;
}

bool ConcreteTransformComponent::validate() const {
    // Check for NaN or invalid values
    for (int i = 0; i < 3; ++i) {
        if (transform.position[i] != transform.position[i]) return false; // NaN check
        if (transform.scale[i] <= 0.0f) return false; // Scale must be positive
    }
    
    // Check quaternion is normalized (roughly)
    float quat_mag_sq = transform.rotation[0] * transform.rotation[0] +
                        transform.rotation[1] * transform.rotation[1] +
                        transform.rotation[2] * transform.rotation[2] +
                        transform.rotation[3] * transform.rotation[3];
    
    return quat_mag_sq > 0.9f && quat_mag_sq < 1.1f; // Allow some tolerance
}

void ConcreteTransformComponent::reset() {
    transform.position[0] = transform.position[1] = transform.position[2] = 0.0f;
    transform.rotation[0] = 1.0f; // w
    transform.rotation[1] = transform.rotation[2] = transform.rotation[3] = 0.0f; // x, y, z
    transform.scale[0] = transform.scale[1] = transform.scale[2] = 1.0f;
    enabled = true;
}

std::unique_ptr<interfaces::CPUPhysicsComponent> ConcreteTransformComponent::clone() const {
    return std::make_unique<ConcreteTransformComponent>(transform);
}

size_t ConcreteTransformComponent::getDataSize() const {
    return sizeof(TransformComponent);
}

// ConcretePhysicsComponent implementation
ConcretePhysicsComponent::ConcretePhysicsComponent(const PhysicsComponent& physics)
    : physics(physics) {
}

ConcretePhysicsComponent::ConcretePhysicsComponent(PhysicsComponent&& physics)
    : physics(std::move(physics)) {
}

interfaces::CPUPhysicsComponent::ComponentType ConcretePhysicsComponent::getType() const {
    return ComponentType::PHYSICS;
}

const char* ConcretePhysicsComponent::getTypeName() const {
    return "ConcretePhysicsComponent";
}

bool ConcretePhysicsComponent::isEnabled() const {
    return enabled;
}

void ConcretePhysicsComponent::setEnabled(bool enabled) {
    this->enabled = enabled;
}

bool ConcretePhysicsComponent::validate() const {
    // Check for valid mass
    if (physics.mass < 0.0f) return false;
    
    // Check for NaN values
    for (int i = 0; i < 3; ++i) {
        if (physics.velocity[i] != physics.velocity[i]) return false; // NaN check
        if (physics.angularVelocity[i] != physics.angularVelocity[i]) return false; // NaN check
    }
    
    // Check material properties
    if (physics.restitution < 0.0f || physics.restitution > 1.0f) return false;
    if (physics.friction < 0.0f) return false;
    
    // Check inverse mass consistency
    if (physics.isStatic && physics.invMass != 0.0f) return false;
    if (!physics.isStatic && physics.mass > 0.0f && physics.invMass <= 0.0f) return false;
    
    return true;
}

void ConcretePhysicsComponent::reset() {
    physics.velocity[0] = physics.velocity[1] = physics.velocity[2] = 0.0f;
    physics.angularVelocity[0] = physics.angularVelocity[1] = physics.angularVelocity[2] = 0.0f;
    physics.mass = 1.0f;
    physics.invMass = 1.0f;
    physics.restitution = 0.5f;
    physics.friction = 0.3f;
    physics.isStatic = false;
    physics.useGravity = true;
    enabled = true;
}

std::unique_ptr<interfaces::CPUPhysicsComponent> ConcretePhysicsComponent::clone() const {
    return std::make_unique<ConcretePhysicsComponent>(physics);
}

size_t ConcretePhysicsComponent::getDataSize() const {
    return sizeof(PhysicsComponent);
}

// ConcreteBoxColliderComponent implementation
ConcreteBoxColliderComponent::ConcreteBoxColliderComponent(const BoxColliderComponent& collider)
    : collider(collider) {
}

ConcreteBoxColliderComponent::ConcreteBoxColliderComponent(BoxColliderComponent&& collider)
    : collider(std::move(collider)) {
}

interfaces::CPUPhysicsComponent::ComponentType ConcreteBoxColliderComponent::getType() const {
    return ComponentType::BOX_COLLIDER;
}

const char* ConcreteBoxColliderComponent::getTypeName() const {
    return "ConcreteBoxColliderComponent";
}

bool ConcreteBoxColliderComponent::isEnabled() const {
    return collider.enabled;
}

void ConcreteBoxColliderComponent::setEnabled(bool enabled) {
    collider.enabled = enabled;
}

bool ConcreteBoxColliderComponent::validate() const {
    // Check for positive dimensions
    if (collider.width <= 0.0f || collider.height <= 0.0f || collider.depth <= 0.0f) {
        return false;
    }
    
    // Check for NaN values
    if (collider.width != collider.width ||
        collider.height != collider.height ||
        collider.depth != collider.depth) {
        return false;
    }
    
    return true;
}

void ConcreteBoxColliderComponent::reset() {
    collider.width = collider.height = collider.depth = 1.0f;
    collider.enabled = true;
}

std::unique_ptr<interfaces::CPUPhysicsComponent> ConcreteBoxColliderComponent::clone() const {
    return std::make_unique<ConcreteBoxColliderComponent>(collider);
}

size_t ConcreteBoxColliderComponent::getDataSize() const {
    return sizeof(BoxColliderComponent);
}

} // namespace concrete
} // namespace cpu_physics