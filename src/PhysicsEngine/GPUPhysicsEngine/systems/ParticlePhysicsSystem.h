#pragma once

#include <memory>

namespace gpu_physics {

// Forward declarations
class ECSManager;
class VulkanManager;

/**
 * Particle Physics System for GPU-based particle simulation.
 * 
 * This system handles:
 * - Uploading particle data to GPU
 * - Running GPU physics simulation
 * - Downloading data back to CPU (only for components with cpu_offloading enabled)
 * - Managing GPU/CPU data synchronization
 * 
 * The system uses static cpu_offloading flags per component type to determine
 * whether to perform expensive CPU offloading operations.
 */
class ParticlePhysicsSystem {
public:
    ParticlePhysicsSystem();
    ~ParticlePhysicsSystem() = default;
    
    // System lifecycle
    bool initialize();
    void cleanup();
    bool isInitialized() const { return initialized; }
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // GPU data operations
    void uploadParticlesToGPU();
    void downloadParticlesFromGPU();
    void updateUniformBuffer(float deltaTime);
    
    // Configuration
    void setECSManager(std::shared_ptr<ECSManager> ecsManager);
    void setVulkanManager(std::shared_ptr<VulkanManager> vulkanManager);

private:
    bool initialized = false;
    
    // Managers
    std::shared_ptr<ECSManager> ecsManager;
    std::shared_ptr<VulkanManager> vulkanManager;
    
    // Gravity configuration
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;
    
    // Internal methods
    void uploadRegularParticles();
    void uploadCPUOffloadingParticles();
    void downloadCPUOffloadingParticles();
    bool shouldPerformCPUOffloading() const;
};

} // namespace gpu_physics