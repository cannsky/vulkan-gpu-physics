#include "RigidbodyComponentFactory.h"
#include "../../managers/logmanager/Logger.h"
#include <cmath>

namespace cpu_physics {

RigidBodyComponentFactory::RigidBodyComponentFactory() {
    LOG_INFO(LogCategory::PHYSICS, "Creating RigidBody Component Factory");
}

TransformComponent RigidBodyComponentFactory::createTransformComponent(
    float x, float y, float z,
    float sx, float sy, float sz) {
    
    TransformComponent transform;
    transform.position[0] = x;
    transform.position[1] = y;
    transform.position[2] = z;
    
    transform.scale[0] = sx;
    transform.scale[1] = sy;
    transform.scale[2] = sz;
    
    initializeQuaternion(transform.rotation);
    
    return transform;
}

PhysicsComponent RigidBodyComponentFactory::createPhysicsComponent(
    float mass,
    bool isStatic,
    bool useGravity,
    float restitution,
    float friction) {
    
    PhysicsComponent physics;
    physics.mass = mass;
    physics.isStatic = isStatic;
    physics.useGravity = useGravity;
    physics.restitution = restitution;
    physics.friction = friction;
    
    // Initialize velocities to zero
    physics.velocity[0] = physics.velocity[1] = physics.velocity[2] = 0.0f;
    physics.angularVelocity[0] = physics.angularVelocity[1] = physics.angularVelocity[2] = 0.0f;
    
    calculateInverseMass(physics);
    
    return physics;
}

BoxColliderComponent RigidBodyComponentFactory::createBoxColliderComponent(
    float width,
    float height,
    float depth,
    bool enabled) {
    
    BoxColliderComponent collider;
    collider.width = width;
    collider.height = height;
    collider.depth = depth;
    collider.enabled = enabled;
    
    return collider;
}

TransformComponent RigidBodyComponentFactory::createTransformAt(float x, float y, float z) {
    return createTransformComponent(x, y, z, 1.0f, 1.0f, 1.0f);
}

PhysicsComponent RigidBodyComponentFactory::createStaticPhysics() {
    return createPhysicsComponent(0.0f, true, false, 0.3f, 0.8f);
}

PhysicsComponent RigidBodyComponentFactory::createDynamicPhysics(float mass) {
    return createPhysicsComponent(mass, false, true, 0.5f, 0.3f);
}

BoxColliderComponent RigidBodyComponentFactory::createBoxCollider(float width, float height, float depth) {
    return createBoxColliderComponent(width, height, depth, true);
}

bool RigidBodyComponentFactory::validateTransformComponent(const TransformComponent& transform) {
    // Check for NaN or infinite values
    for (int i = 0; i < 3; i++) {
        if (!std::isfinite(transform.position[i]) || 
            !std::isfinite(transform.scale[i])) {
            return false;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        if (!std::isfinite(transform.rotation[i])) {
            return false;
        }
    }
    
    // Check for zero or negative scale
    if (transform.scale[0] <= 0.0f || transform.scale[1] <= 0.0f || transform.scale[2] <= 0.0f) {
        return false;
    }
    
    return true;
}

bool RigidBodyComponentFactory::validatePhysicsComponent(const PhysicsComponent& physics) {
    // Check for valid mass (non-negative)
    if (physics.mass < 0.0f || !std::isfinite(physics.mass)) {
        return false;
    }
    
    // Check for finite velocities
    for (int i = 0; i < 3; i++) {
        if (!std::isfinite(physics.velocity[i]) || 
            !std::isfinite(physics.angularVelocity[i])) {
            return false;
        }
    }
    
    // Check for valid material properties
    if (physics.restitution < 0.0f || physics.restitution > 1.0f ||
        physics.friction < 0.0f || physics.friction > 1.0f) {
        return false;
    }
    
    return true;
}

bool RigidBodyComponentFactory::validateBoxColliderComponent(const BoxColliderComponent& collider) {
    // Check for positive dimensions
    if (collider.width <= 0.0f || collider.height <= 0.0f || collider.depth <= 0.0f) {
        return false;
    }
    
    // Check for finite values
    if (!std::isfinite(collider.width) || 
        !std::isfinite(collider.height) || 
        !std::isfinite(collider.depth)) {
        return false;
    }
    
    return true;
}

void RigidBodyComponentFactory::setTransformPosition(TransformComponent& transform, float x, float y, float z) {
    transform.position[0] = x;
    transform.position[1] = y;
    transform.position[2] = z;
}

void RigidBodyComponentFactory::setTransformScale(TransformComponent& transform, float sx, float sy, float sz) {
    transform.scale[0] = sx;
    transform.scale[1] = sy;
    transform.scale[2] = sz;
}

void RigidBodyComponentFactory::setPhysicsVelocity(PhysicsComponent& physics, float vx, float vy, float vz) {
    physics.velocity[0] = vx;
    physics.velocity[1] = vy;
    physics.velocity[2] = vz;
}

void RigidBodyComponentFactory::updatePhysicsMass(PhysicsComponent& physics, float mass) {
    physics.mass = mass;
    calculateInverseMass(physics);
}

void RigidBodyComponentFactory::initializeQuaternion(float* quaternion) {
    // Initialize to identity quaternion (no rotation)
    quaternion[0] = 1.0f; // w
    quaternion[1] = 0.0f; // x
    quaternion[2] = 0.0f; // y
    quaternion[3] = 0.0f; // z
}

void RigidBodyComponentFactory::calculateInverseMass(PhysicsComponent& physics) {
    if (physics.isStatic || physics.mass <= 0.0f) {
        physics.invMass = 0.0f;
    } else {
        physics.invMass = 1.0f / physics.mass;
    }
}

} // namespace cpu_physics