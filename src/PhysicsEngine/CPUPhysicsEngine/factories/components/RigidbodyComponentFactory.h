#pragma once

#include "../../components.h" // For component definitions (now split under components/)
#include <memory>

namespace cpu_physics {

// Forward declaration
class ECSManager;

class RigidBodyComponentFactory {
public:
    RigidBodyComponentFactory();
    ~RigidBodyComponentFactory() = default;
    static TransformComponent createTransformComponent(
        float x = 0.0f, float y = 0.0f, float z = 0.0f,
        float sx = 1.0f, float sy = 1.0f, float sz = 1.0f
    );
    static PhysicsComponent createPhysicsComponent(
        float mass = 1.0f,
        bool isStatic = false,
        bool useGravity = true,
        float restitution = 0.5f,
        float friction = 0.3f
    );
    static BoxColliderComponent createBoxColliderComponent(
        float width = 1.0f,
        float height = 1.0f,
        float depth = 1.0f,
        bool enabled = true
    );
    static TransformComponent createTransformAt(float x, float y, float z);
    static PhysicsComponent createStaticPhysics();
    static PhysicsComponent createDynamicPhysics(float mass = 1.0f);
    static BoxColliderComponent createBoxCollider(float width, float height, float depth);
    static bool validateTransformComponent(const TransformComponent& transform);
    static bool validatePhysicsComponent(const PhysicsComponent& physics);
    static bool validateBoxColliderComponent(const BoxColliderComponent& collider);
    static void setTransformPosition(TransformComponent& transform, float x, float y, float z);
    static void setTransformScale(TransformComponent& transform, float sx, float sy, float sz);
    static void setPhysicsVelocity(PhysicsComponent& physics, float vx, float vy, float vz);
    static void updatePhysicsMass(PhysicsComponent& physics, float mass);
private:
    static void initializeQuaternion(float* quaternion);
    static void calculateInverseMass(PhysicsComponent& physics);
};

} // namespace cpu_physics
