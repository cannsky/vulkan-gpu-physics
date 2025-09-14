#pragma once

#include <memory>
#include <vector>

// Forward declarations
#ifdef VULKAN_AVAILABLE
namespace gpu_physics {
    class GPUPhysicsSystem;
}
#endif

namespace cpu_physics {
    class CPUPhysicsSystem;
    class RigidBodyComponent;
}

/**
 * Titanium Physics Engine - Hybrid GPU/CPU Physics System
 * 
 * This is the main orchestrator for the titanium-physics system that manages:
 * - GPU physics for particles and fluid simulations (when Vulkan is available)
 * - CPU physics for complex rigidbody operations with ECS architecture
 */
class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();
    
    // Initialization
    bool initialize(uint32_t maxParticles = 1024, uint32_t maxRigidBodies = 512);
    void cleanup();
    bool isInitialized() const { return initialized; }
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // GPU Physics (Particles/Fluids) Interface - only available with Vulkan
    bool addParticle(float x, float y, float z, float vx = 0.0f, float vy = 0.0f, float vz = 0.0f, float mass = 1.0f);
    size_t getParticleCount() const;
    
    // CPU Physics (RigidBodies) Interface - always available
    uint32_t createRigidBody(float x, float y, float z, float width, float height, float depth, float mass = 1.0f, uint32_t layer = 0);
    bool removeRigidBody(uint32_t bodyId);
    cpu_physics::RigidBodyComponent* getRigidBody(uint32_t bodyId);
    
    // Layer system for collision filtering
    uint32_t createPhysicsLayer(const std::string& name);
    bool setLayerInteraction(uint32_t layer1, uint32_t layer2, bool canInteract);
    bool canLayersInteract(uint32_t layer1, uint32_t layer2) const;
    
    // Subsystem accessors
#ifdef VULKAN_AVAILABLE
    gpu_physics::GPUPhysicsSystem* getGPUPhysics() const { return gpuPhysics.get(); }
#endif
    cpu_physics::CPUPhysicsSystem* getCPUPhysics() const { return cpuPhysics.get(); }

private:
    bool initialized = false;
    
#ifdef VULKAN_AVAILABLE
    std::unique_ptr<gpu_physics::GPUPhysicsSystem> gpuPhysics;
#endif
    std::unique_ptr<cpu_physics::CPUPhysicsSystem> cpuPhysics;
    
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;
};