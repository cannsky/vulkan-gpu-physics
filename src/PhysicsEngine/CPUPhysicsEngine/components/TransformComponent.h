#pragma once

namespace cpu_physics {

// Transform Component
struct TransformComponent {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // quaternion (w, x, y, z)
    float scale[3] = {1.0f, 1.0f, 1.0f};
};

} // namespace cpu_physics
