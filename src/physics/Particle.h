#pragma once

struct Particle {
    float position[3];
    float velocity[3];
    float mass;
    float padding; // For alignment
};