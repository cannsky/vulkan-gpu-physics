#pragma once

#include "../BaseManager.h"
#include "../../physics/Particle.h"
#include <vector>
#include <memory>

// Forward declarations
class ParticleSystem;

/**
 * Manager for particle physics system.
 * Provides singleton access to particle operations.
 */
class ParticleManager : public BaseManager {
public:
    static ParticleManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Particle operations
    bool addParticle(const Particle& particle);
    std::vector<Particle> getParticles() const;
    uint32_t getParticleCount() const;
    uint32_t getMaxParticles() const;
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // GPU operations
    void uploadParticlesToGPU();
    void downloadParticlesFromGPU();

private:
    ParticleManager() = default;
    ~ParticleManager() = default;
    
    bool initialized = false;
    std::shared_ptr<ParticleSystem> particleSystem;
};