#pragma once

#include <cstdint>

// Mock RigidBody structure for testing
struct RigidBody {
    float position[3];
    float velocity[3];
    float angularVelocity[3];
    float rotation[4];
    float mass;
    float invMass;
    float inertia[3];
    float invInertia[3];
    float restitution;
    float friction;
    uint32_t shapeType;
    float shapeData[4];
    uint32_t isStatic;
    float padding[3];
};

enum class RigidBodyShape : uint32_t {
    SPHERE = 0,
    BOX = 1,
    CAPSULE = 2
};