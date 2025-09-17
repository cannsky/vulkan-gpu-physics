#pragma once

#include <cstdint>

// NOTE: This entity struct is shared by CPU and GPU workers for collision helpers
struct RigidBody {
    float position[3];
    float velocity[3];
    float angularVelocity[3];
    float rotation[4]; // quaternion (w, x, y, z)
    float mass;
    float invMass; // 1/mass, or 0 for static bodies
    float inertia[3]; // diagonal inertia tensor
    float invInertia[3]; // inverse inertia tensor
    float restitution; // elasticity
    float friction;
    uint32_t shapeType; // 0=sphere, 1=box, 2=capsule
    float shapeData[4]; // shape-specific data (radius, half-extents, etc.)
    uint32_t isStatic; // 0=dynamic, 1=static
    uint32_t layer; // Physics layer for collision filtering
    float padding[2]; // For alignment
};

// Shape types
enum class RigidBodyShape : uint32_t {
    SPHERE = 0,
    BOX = 1,
    CAPSULE = 2,
    PLANE = 3
};
