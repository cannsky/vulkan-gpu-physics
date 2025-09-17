#pragma once

namespace cpu_physics {

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

} // namespace cpu_physics
