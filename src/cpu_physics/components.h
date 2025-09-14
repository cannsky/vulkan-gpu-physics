#pragma once

#include <cstdint>

namespace cpu_physics {

// Box Collider Component (only supported collider type for now)
struct BoxColliderComponent {
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    bool enabled = true;
};

// Transform Component
struct TransformComponent {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // quaternion (w, x, y, z)
    float scale[3] = {1.0f, 1.0f, 1.0f};
};

// Physics Component
struct PhysicsComponent {
    float velocity[3] = {0.0f, 0.0f, 0.0f};
    float angularVelocity[3] = {0.0f, 0.0f, 0.0f};
    float mass = 1.0f;
    float invMass = 1.0f; // 1/mass, or 0 for static bodies
    float restitution = 0.5f; // elasticity
    float friction = 0.3f;
    bool isStatic = false;
    bool useGravity = true;
};

// Complete RigidBody entity with all components (legacy compatibility)
struct RigidBodyComponent {
    uint32_t entityId;
    TransformComponent transform;
    PhysicsComponent physics;
    BoxColliderComponent collider;
    uint32_t layer = 0; // Physics layer for collision filtering
    bool hasCollider = true;
};

} // namespace cpu_physics