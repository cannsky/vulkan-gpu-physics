#pragma once

#include <vulkan/vulkan.h>

struct Contact {
    float position[3];    // Contact point in world space
    float normal[3];      // Contact normal (from body A to body B)
    float penetration;    // Penetration depth
    uint32_t bodyIdA;     // First rigid body ID
    uint32_t bodyIdB;     // Second rigid body ID
    float restitution;    // Combined restitution
    float friction;       // Combined friction
    float padding[2];     // For alignment
};

struct CollisionPair {
    uint32_t bodyIdA;
    uint32_t bodyIdB;
    uint32_t isActive;    // 1 if collision, 0 if separated
    uint32_t padding;     // For alignment
};