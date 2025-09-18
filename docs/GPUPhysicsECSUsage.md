# GPU Physics ECS System Usage Example

## Overview

The new GPU physics system uses an ECS (Entity-Component-System) architecture optimized for GPU particle simulation. The key feature is **static CPU offloading** configuration at the component level.

## Key Components

### 1. ParticleComponent (No CPU Offloading)
```cpp
#include "src/PhysicsEngine/GPUPhysicsEngine/components/ParticleComponent.h"

// Regular particle component - stays on GPU
gpu_physics::ParticleComponent particle;
particle.position[0] = 1.0f;
particle.position[1] = 2.0f;
particle.position[2] = 3.0f;
particle.velocity[0] = 0.1f;
particle.velocity[1] = 0.2f;
particle.velocity[2] = 0.3f;
particle.mass = 1.5f;

// Static flag indicates no CPU offloading needed
static_assert(!gpu_physics::ParticleComponent::cpu_offloading);
```

### 2. ParticleComponentWithCPUOffloading (CPU Offloading Enabled)
```cpp
// CPU offloading particle component - data will be downloaded back to CPU
gpu_physics::ParticleComponentWithCPUOffloading cpuParticle;
cpuParticle.position[0] = 4.0f;
cpuParticle.position[1] = 5.0f;
cpuParticle.position[2] = 6.0f;
cpuParticle.velocity[0] = 0.4f;
cpuParticle.velocity[1] = 0.5f;
cpuParticle.velocity[2] = 0.6f;
cpuParticle.mass = 2.0f;

// Static flag indicates CPU offloading is enabled
static_assert(gpu_physics::ParticleComponentWithCPUOffloading::cpu_offloading);
```

## Usage Example

```cpp
#include "src/PhysicsEngine/GPUPhysicsEngine/managers/physicsmanager/GPUPhysicsManager.h"

// Get GPU Physics Manager
auto& gpuPhysics = GPUPhysicsManager::getInstance();
if (!gpuPhysics.initialize()) {
    // Handle initialization failure
}

// Get ECS Manager
auto ecsManager = gpuPhysics.getECSManager();
auto particleSystem = gpuPhysics.getParticlePhysicsSystem();

// Create entities
uint32_t regularEntity = ecsManager->createEntity();
uint32_t cpuOffloadEntity = ecsManager->createEntity();

// Add regular particle (no CPU offloading)
gpu_physics::ParticleComponent regularParticle;
regularParticle.position[0] = 0.0f;
regularParticle.position[1] = 10.0f;
regularParticle.position[2] = 0.0f;
regularParticle.mass = 1.0f;
ecsManager->addParticleComponent(regularEntity, regularParticle);

// Add CPU offloading particle
gpu_physics::ParticleComponentWithCPUOffloading cpuParticle;
cpuParticle.position[0] = 5.0f;
cpuParticle.position[1] = 10.0f;
cpuParticle.position[2] = 0.0f;
cpuParticle.mass = 2.0f;
ecsManager->addParticleComponentWithCPUOffloading(cpuOffloadEntity, cpuParticle);

// Run physics simulation
float deltaTime = 0.016f; // 60 FPS
gpuPhysics.updatePhysics(deltaTime);

// The system automatically:
// 1. Uploads all particle data to GPU
// 2. Runs GPU physics simulation
// 3. Downloads ONLY CPU offloading particles back to CPU (if any exist)
```

## Performance Benefits

### Static CPU Offloading Configuration

The system uses **static constexpr flags** per component type instead of per-particle boolean values:

```cpp
// OLD (per-particle check - SLOW):
for (auto& particle : particles) {
    if (particle.cpu_offloading) {  // Check every particle
        downloadFromGPU(particle);
    }
}

// NEW (component-level check - FAST):
if (ecsManager->getParticleComponentWithCPUOffloadingCount() > 0) {
    // Only check once per component type, then download all
    downloadAllCPUOffloadingParticles();
}
```

### Benefits:
1. **No per-particle checks** during physics updates
2. **Batch operations** for GPU data transfer
3. **Static optimization** - compiler can optimize away unused code paths
4. **Memory efficiency** - no extra boolean per particle

## Architecture Changes

### Removed Components:
- ❌ CollisionManager (GPU no longer handles collisions)
- ❌ ParticleManager (replaced with ECS system)

### New Components:
- ✅ gpu_physics::ECSManager (entity and component management)
- ✅ gpu_physics::ParticlePhysicsSystem (GPU data transfer and simulation)
- ✅ ParticleComponent types with static cpu_offloading flags

## Integration with CPU Physics

Particles with `cpu_offloading = true` can interact with the CPU physics system:

```cpp
// Create particles that need CPU interaction
auto cpuInteractionEntities = ecsManager->getEntitiesWithParticleComponentWithCPUOffloading();

for (uint32_t entityId : cpuInteractionEntities) {
    auto* particle = ecsManager->getParticleComponentWithCPUOffloading(entityId);
    if (particle) {
        // This particle data is automatically kept in sync with CPU
        // Can be used for collision with CPU rigidbodies
    }
}
```

This design ensures maximum performance while maintaining the flexibility to have certain particles interact with the CPU physics system when needed.