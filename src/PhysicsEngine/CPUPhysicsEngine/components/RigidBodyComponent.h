#pragma once

#include <cstdint>
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "BoxColliderComponent.h"

namespace cpu_physics {

// Legacy compatibility wrapper combining core components for a rigid body
struct RigidBodyComponent {
    uint32_t entityId;
    TransformComponent transform;
    PhysicsComponent physics;
    BoxColliderComponent collider;
    uint32_t layer = 0; // Physics layer for collision filtering
    bool hasCollider = true;
};

} // namespace cpu_physics
