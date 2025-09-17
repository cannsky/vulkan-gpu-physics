#pragma once

namespace cpu_physics {

// Box Collider Component (only supported collider type for now)
struct BoxColliderComponent {
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    bool enabled = true;
};

} // namespace cpu_physics
