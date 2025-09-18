#pragma once

namespace gpu_physics {

/**
 * Particle Component for GPU-based particle physics system.
 * Contains particle physics data and CPU offloading configuration.
 */
struct ParticleComponent {
    // Particle physics data
    float position[3] = {0.0f, 0.0f, 0.0f};
    float velocity[3] = {0.0f, 0.0f, 0.0f};
    float mass = 1.0f;
    
    // Static CPU offloading flag per component type
    // This determines if ALL particles of this component type should be offloaded to CPU
    // This is a static configuration per component, not per individual particle instance
    static constexpr bool cpu_offloading = false;
    
    // Padding for GPU buffer alignment
    float padding = 0.0f;
};

/**
 * Specialized particle component with CPU offloading enabled.
 * Use this component type when particles need to interact with CPU physics.
 */
struct ParticleComponentWithCPUOffloading {
    // Particle physics data
    float position[3] = {0.0f, 0.0f, 0.0f};
    float velocity[3] = {0.0f, 0.0f, 0.0f};
    float mass = 1.0f;
    
    // Static CPU offloading flag - enabled for this component type
    static constexpr bool cpu_offloading = true;
    
    // Padding for GPU buffer alignment
    float padding = 0.0f;
};

} // namespace gpu_physics