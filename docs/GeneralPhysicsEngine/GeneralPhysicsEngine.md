# General Physics Engine

The General Physics Engine (Titanium Physics Engine) is the main orchestrator that coordinates both CPU and GPU physics systems in a unified hybrid architecture.

## Overview

The General Physics Engine serves as the primary interface for all physics operations in the Titanium Physics system. It intelligently manages the division of work between CPU-based rigidbody physics and GPU-based particle physics, providing a seamless unified API for applications.

## Architecture Philosophy

### Hybrid Design Strategy

The Titanium Physics Engine implements a **hybrid CPU/GPU approach** that leverages the strengths of both processing units:

- **GPU Physics**: Optimized for massive parallel simulations (particles, fluids)
- **CPU Physics**: Optimized for complex interactions and game logic (rigidbodies, constraints)
- **Unified Interface**: Single API that abstracts the underlying complexity

### Design Principles

1. **Separation of Concerns**: Each subsystem handles what it does best
2. **Performance Optimization**: Leverage appropriate hardware for each physics type
3. **Seamless Integration**: Unified API hides implementation complexity
4. **Extensibility**: Easy to add new physics systems or capabilities
5. **Backward Compatibility**: Maintains familiar physics API patterns

## Core Architecture

### Main PhysicsEngine Class

**Location**: `src/physics_engine/physics_engine.h`

```cpp
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
    
    // Initialization and cleanup
    bool initialize(uint32_t maxParticles = 0, uint32_t maxRigidBodies = 512);
    void cleanup();
    bool isInitialized() const { return initialized; }
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // CPU Physics (RigidBodies)
    uint32_t createRigidBody(float x, float y, float z, 
                           float width, float height, float depth, 
                           float mass = 1.0f, uint32_t layer = 0);
    bool removeRigidBody(uint32_t entityId);
    RigidBodyComponent* getRigidBody(uint32_t entityId);
    
    // GPU Physics (Particles) - when available
#ifdef VULKAN_AVAILABLE
    bool addParticle(float x, float y, float z, 
                    float vx = 0.0f, float vy = 0.0f, float vz = 0.0f, 
                    float mass = 1.0f);
    std::vector<Particle> getParticles() const;
    size_t getParticleCount() const;
#endif
    
    // Layer system
    uint32_t createPhysicsLayer(const std::string& name);
    bool setLayerInteraction(uint32_t layer1, uint32_t layer2, bool canInteract);
    
    // System access
#ifdef VULKAN_AVAILABLE
    gpu_physics::GPUPhysicsEngine* getGPUPhysics() const { return gpuPhysics.get(); }
#endif
    cpu_physics::CPUPhysicsEngine* getCPUPhysics() const { return cpuPhysics.get(); }
    
private:
    bool initialized = false;
    
#ifdef VULKAN_AVAILABLE
    std::unique_ptr<gpu_physics::GPUPhysicsEngine> gpuPhysics;
#endif
    std::unique_ptr<cpu_physics::CPUPhysicsEngine> cpuPhysics;
    
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;
};
```

## Hybrid System Coordination

### Work Division Strategy

The Physics Engine intelligently divides physics simulation work between CPU and GPU based on the nature of the physics objects:

#### CPU Physics Responsibilities
- **Rigidbody Dynamics**: Complex rigidbody interactions with realistic physics
- **Collision Response**: Detailed collision detection and response
- **Constraint Systems**: Joints, springs, and mechanical constraints
- **Game Logic Integration**: Physics that needs tight coupling with gameplay
- **Complex Shapes**: Non-trivial collision geometries

#### GPU Physics Responsibilities  
- **Particle Systems**: Thousands of simple particles (explosions, weather, debris)
- **Fluid Simulation**: Liquid and gas dynamics using particle methods
- **Simple Physics**: Massively parallel simple physics calculations
- **Visual Effects**: Physics-based visual effects and simulations

### Coordination Mechanisms

#### Unified Initialization
```cpp
bool PhysicsEngine::initialize(uint32_t maxParticles, uint32_t maxRigidBodies) {
    LOG_INFO(LogCategory::PHYSICS, "Initializing Titanium Physics Engine");
    LOG_INFO(LogCategory::PHYSICS, "Initializing hybrid physics system - GPU: " + 
             std::to_string(maxParticles) + " particles, CPU: " + 
             std::to_string(maxRigidBodies) + " rigidbodies");
    
    // Always initialize CPU physics
    cpuPhysics = std::make_unique<cpu_physics::CPUPhysicsEngine>();
    if (!cpuPhysics->initialize(maxRigidBodies)) {
        LOG_ERROR(LogCategory::PHYSICS, "Failed to initialize CPU physics system");
        return false;
    }
    
#ifdef VULKAN_AVAILABLE
    // Initialize GPU physics if Vulkan is available
    if (maxParticles > 0) {
        auto vulkanContext = std::make_shared<VulkanContext>();
        if (vulkanContext->initialize()) {
            gpuPhysics = std::make_unique<gpu_physics::GPUPhysicsEngine>(vulkanContext);
            if (!gpuPhysics->initialize(maxParticles)) {
                LOG_WARNING(LogCategory::PHYSICS, "Failed to initialize GPU physics, continuing with CPU only");
                gpuPhysics.reset();
            }
        }
    }
#else
    LOG_INFO(LogCategory::PHYSICS, "Vulkan not compiled in, GPU physics disabled");
#endif
    
    // Set synchronized gravity
    setGravity(gravity.x, gravity.y, gravity.z);
    
    initialized = true;
    LOG_INFO(LogCategory::PHYSICS, "Titanium Physics Engine successfully initialized");
    return true;
}
```

#### Synchronized Updates
```cpp
void PhysicsEngine::updatePhysics(float deltaTime) {
    if (!initialized) return;
    
    // Update both physics systems in parallel
    // (In a real implementation, these could run on separate threads)
    
#ifdef VULKAN_AVAILABLE
    if (gpuPhysics) {
        gpuPhysics->updatePhysics(deltaTime);
    }
#endif
    
    if (cpuPhysics) {
        cpuPhysics->updatePhysics(deltaTime);
    }
}
```

#### Synchronized Configuration
```cpp
void PhysicsEngine::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    // Synchronize gravity across all physics systems
#ifdef VULKAN_AVAILABLE
    if (gpuPhysics) {
        gpuPhysics->setGravity(x, y, z);
    }
#endif
    
    if (cpuPhysics) {
        cpuPhysics->setGravity(x, y, z);
    }
    
    LOG_INFO(LogCategory::PHYSICS, "Gravity set to (" + 
             std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}
```

## Unified API Design

### Consistent Interface Patterns

The Physics Engine provides consistent API patterns regardless of the underlying implementation:

#### Object Creation
```cpp
// RigidBodies (CPU Physics)
uint32_t rigidBodyId = physicsEngine.createRigidBody(
    0.0f, 10.0f, 0.0f,    // position
    2.0f, 2.0f, 2.0f,     // dimensions
    1.0f,                 // mass
    0                     // layer
);

// Particles (GPU Physics)
#ifdef VULKAN_AVAILABLE
bool success = physicsEngine.addParticle(
    0.0f, 10.0f, 0.0f,    // position
    0.0f, 0.0f, 0.0f,     // velocity
    1.0f                  // mass
);
#endif
```

#### Data Access
```cpp
// Access CPU physics objects
RigidBodyComponent* rigidBody = physicsEngine.getRigidBody(rigidBodyId);
if (rigidBody) {
    float x = rigidBody->transform.position[0];
    float y = rigidBody->transform.position[1];
    float z = rigidBody->transform.position[2];
}

// Access GPU physics objects
#ifdef VULKAN_AVAILABLE
auto particles = physicsEngine.getParticles();
for (const auto& particle : particles) {
    float x = particle.position[0];
    float y = particle.position[1];
    float z = particle.position[2];
}
#endif
```

#### Configuration Management
```cpp
// Layer system works across both physics systems
uint32_t playerLayer = physicsEngine.createPhysicsLayer("Player");
uint32_t enemyLayer = physicsEngine.createPhysicsLayer("Enemy");
physicsEngine.setLayerInteraction(playerLayer, enemyLayer, true);

// Global settings affect both systems
physicsEngine.setGravity(0.0f, -9.81f, 0.0f);
```

## Build System Integration

### Conditional Compilation

The Physics Engine uses conditional compilation to handle Vulkan availability:

```cpp
// CMakeLists.txt configuration
if(VULKAN_FOUND)
    target_compile_definitions(titanium-physics PUBLIC VULKAN_AVAILABLE)
    target_link_libraries(titanium-physics ${Vulkan_LIBRARIES})
else()
    message(STATUS "Vulkan not found - building CPU-only physics system")
endif()
```

### Graceful Degradation
```cpp
// Runtime adaptation to available systems
bool PhysicsEngine::initialize(uint32_t maxParticles, uint32_t maxRigidBodies) {
    // CPU physics is always available
    initializeCPUPhysics(maxRigidBodies);
    
#ifdef VULKAN_AVAILABLE
    // GPU physics is optional
    if (maxParticles > 0 && isVulkanAvailable()) {
        initializeGPUPhysics(maxParticles);
    }
#endif
    
    // System works with whatever is available
    return true;
}
```

## Performance Optimization

### System-Specific Optimizations

Each subsystem is optimized for its specific use case:

#### CPU Physics Optimizations
- **ECS Architecture**: Cache-friendly component storage
- **Spatial Partitioning**: Efficient collision detection
- **SIMD Instructions**: Vectorized math operations
- **Multi-threading**: Parallel processing of independent operations

#### GPU Physics Optimizations
- **Compute Shaders**: Massively parallel GPU computation
- **Memory Coalescing**: Optimal GPU memory access patterns
- **Workgroup Optimization**: Efficient GPU utilization
- **Minimal CPU-GPU Transfer**: Reduce data transfer overhead

### Hybrid Optimizations

#### Load Balancing
```cpp
class LoadBalancer {
public:
    void balancePhysicsLoad() {
        // Analyze current workload
        float cpuUtilization = measureCPUPhysicsLoad();
        float gpuUtilization = measureGPUPhysicsLoad();
        
        // Adjust work distribution if needed
        if (cpuUtilization > 0.8f && gpuUtilization < 0.5f) {
            // Move some work to GPU if possible
            migrateWorkToGPU();
        }
    }
    
private:
    void migrateWorkToGPU() {
        // Convert some simple rigidbodies to particles
        // Or offload some calculations to GPU compute shaders
    }
};
```

#### Memory Management
```cpp
class HybridMemoryManager {
public:
    // Separate memory pools for different physics types
    struct MemoryPools {
        // CPU physics memory
        std::unique_ptr<ECSMemoryPool> ecsPool;
        std::unique_ptr<CollisionMemoryPool> collisionPool;
        
        // GPU physics memory
        std::unique_ptr<VulkanMemoryPool> vulkanPool;
        std::unique_ptr<BufferMemoryPool> bufferPool;
    } memoryPools;
    
    void optimizeMemoryLayout() {
        // Optimize memory layout for both CPU and GPU access
        memoryPools.ecsPool->optimizeComponentLayout();
        memoryPools.vulkanPool->optimizeBufferLayout();
    }
};
```

## Advanced Features

### Cross-System Interactions

While CPU and GPU physics systems primarily operate independently, the hybrid architecture supports limited cross-system interactions:

#### Collision Events
```cpp
class CrossSystemCollisionManager {
public:
    struct CollisionEvent {
        enum Type { RIGIDBODY_PARTICLE, PARTICLE_RIGIDBODY } type;
        uint32_t rigidBodyId;
        uint32_t particleId;
        float contactPoint[3];
        float contactNormal[3];
    };
    
    void processRigidBodyParticleCollisions() {
        // Check collisions between CPU rigidbodies and GPU particles
        auto rigidBodies = cpuPhysics->getAllRigidBodies();
        auto particles = gpuPhysics->getParticles();
        
        for (const auto& rb : rigidBodies) {
            for (const auto& particle : particles) {
                if (testCollision(rb, particle)) {
                    CollisionEvent event;
                    event.type = CollisionEvent::RIGIDBODY_PARTICLE;
                    event.rigidBodyId = rb.entityId;
                    // Process collision...
                }
            }
        }
    }
};
```

#### Force Application
```cpp
class CrossSystemForceManager {
public:
    // Apply forces from rigidbodies to nearby particles
    void applyRigidBodyInfluence() {
        auto rigidBodies = cpuPhysics->getAllRigidBodies();
        auto particles = gpuPhysics->getParticles();
        
        for (const auto& rb : rigidBodies) {
            for (auto& particle : particles) {
                float distance = calculateDistance(rb.transform.position, particle.position);
                if (distance < influenceRadius) {
                    applyInfluenceForce(rb, particle);
                }
            }
        }
    }
};
```

### Debug and Visualization

#### Unified Debug Interface
```cpp
class PhysicsDebugger {
public:
    struct DebugDrawCommands {
        std::vector<RigidBodyDebugInfo> rigidBodies;
        std::vector<ParticleDebugInfo> particles;
        std::vector<CollisionDebugInfo> collisions;
        std::vector<ForceDebugInfo> forces;
    };
    
    DebugDrawCommands generateDebugCommands() {
        DebugDrawCommands commands;
        
        // Get CPU physics debug info
        if (cpuPhysics) {
            commands.rigidBodies = cpuPhysics->getDebugInfo();
        }
        
        // Get GPU physics debug info
#ifdef VULKAN_AVAILABLE
        if (gpuPhysics) {
            commands.particles = gpuPhysics->getDebugInfo();
        }
#endif
        
        return commands;
    }
};
```

#### Performance Profiling
```cpp
class PhysicsProfiler {
public:
    struct PerformanceMetrics {
        float cpuPhysicsTime;
        float gpuPhysicsTime;
        float totalPhysicsTime;
        
        uint32_t rigidBodyCount;
        uint32_t particleCount;
        uint32_t collisionCount;
        
        float memoryUsageCPU;
        float memoryUsageGPU;
    };
    
    PerformanceMetrics getMetrics() {
        PerformanceMetrics metrics = {};
        
        // Collect CPU metrics
        if (cpuPhysics) {
            metrics.cpuPhysicsTime = cpuPhysics->getLastUpdateTime();
            metrics.rigidBodyCount = cpuPhysics->getRigidBodyCount();
            metrics.memoryUsageCPU = cpuPhysics->getMemoryUsage();
        }
        
        // Collect GPU metrics
#ifdef VULKAN_AVAILABLE
        if (gpuPhysics) {
            metrics.gpuPhysicsTime = gpuPhysics->getLastUpdateTime();
            metrics.particleCount = gpuPhysics->getParticleCount();
            metrics.memoryUsageGPU = gpuPhysics->getMemoryUsage();
        }
#endif
        
        metrics.totalPhysicsTime = metrics.cpuPhysicsTime + metrics.gpuPhysicsTime;
        
        return metrics;
    }
};
```

## Usage Examples

### Basic Hybrid Setup
```cpp
#include "physics_engine.h"

int main() {
    PhysicsEngine physicsEngine;
    
    // Initialize with both CPU and GPU physics
    physicsEngine.initialize(
        1000,  // maxParticles (GPU)
        100    // maxRigidBodies (CPU)
    );
    
    // Create some rigidbodies
    uint32_t ground = physicsEngine.createRigidBody(
        0.0f, -10.0f, 0.0f,    // position
        50.0f, 1.0f, 50.0f,    // size (large flat ground)
        0.0f,                  // mass (0 = static)
        0                      // layer
    );
    
    uint32_t box = physicsEngine.createRigidBody(
        0.0f, 10.0f, 0.0f,     // position
        2.0f, 2.0f, 2.0f,      // size
        1.0f,                  // mass
        0                      // layer
    );
    
    // Add some particles
#ifdef VULKAN_AVAILABLE
    for (int i = 0; i < 100; ++i) {
        physicsEngine.addParticle(
            rand() % 20 - 10.0f,  // random x
            20.0f,                // high y
            rand() % 20 - 10.0f,  // random z
            0.0f, 0.0f, 0.0f,     // zero velocity
            0.1f                  // small mass
        );
    }
#endif
    
    // Main simulation loop
    float deltaTime = 1.0f / 60.0f;
    while (true) {
        // Update physics
        physicsEngine.updatePhysics(deltaTime);
        
        // Get rigidbody data for rendering
        RigidBodyComponent* boxRB = physicsEngine.getRigidBody(box);
        if (boxRB) {
            renderBox(boxRB->transform.position, boxRB->collider);
        }
        
        // Get particle data for rendering
#ifdef VULKAN_AVAILABLE
        auto particles = physicsEngine.getParticles();
        for (const auto& particle : particles) {
            renderParticle(particle.position);
        }
#endif
        
        // Render frame
        swapBuffers();
    }
    
    // Cleanup
    physicsEngine.cleanup();
    return 0;
}
```

### Advanced Configuration
```cpp
void setupComplexPhysicsScene(PhysicsEngine& engine) {
    // Create physics layers for different object types
    uint32_t staticLayer = engine.createPhysicsLayer("Static");
    uint32_t dynamicLayer = engine.createPhysicsLayer("Dynamic");  
    uint32_t particleLayer = engine.createPhysicsLayer("Particles");
    
    // Configure layer interactions
    engine.setLayerInteraction(staticLayer, dynamicLayer, true);   // Static collides with dynamic
    engine.setLayerInteraction(dynamicLayer, dynamicLayer, true);  // Dynamic collides with dynamic
    engine.setLayerInteraction(staticLayer, particleLayer, false); // Static ignores particles
    
    // Custom gravity for different effects
    engine.setGravity(0.0f, -9.81f, 0.0f);  // Earth-like gravity
    
    // Access subsystems for advanced operations
    auto cpuPhysics = engine.getCPUPhysics();
    if (cpuPhysics) {
        // Configure CPU-specific settings
        auto ecsManager = cpuPhysics->getECSManager();
        auto collisionSystem = cpuPhysics->getCollisionSystem();
        // Advanced ECS operations...
    }
    
#ifdef VULKAN_AVAILABLE
    auto gpuPhysics = engine.getGPUPhysics();
    if (gpuPhysics) {
        // Configure GPU-specific settings
        auto bufferManager = gpuPhysics->getBufferManager();
        auto computePipeline = gpuPhysics->getComputePipeline();
        // Advanced GPU operations...
    }
#endif
}
```

## Design Benefits

### Unified Development Experience
- **Single API**: Developers work with one consistent interface
- **Automatic Optimization**: System automatically chooses optimal processing unit
- **Graceful Degradation**: Works on systems without GPU acceleration
- **Future-Proof**: Easy to add new physics systems or acceleration methods

### Performance Benefits
- **Optimal Resource Utilization**: Each processing unit handles its strengths
- **Parallel Processing**: CPU and GPU can work simultaneously on different tasks
- **Reduced Bottlenecks**: Work distribution prevents any single unit from becoming a bottleneck
- **Scalability**: Performance scales with available hardware capabilities

### Maintenance Benefits
- **Modular Design**: Each subsystem can be developed and tested independently
- **Clean Interfaces**: Well-defined boundaries between subsystems
- **Extensibility**: Easy to add new physics capabilities or processing units
- **Debugging**: Clear separation makes debugging and profiling easier

The General Physics Engine provides a powerful, flexible, and performant foundation for physics simulation that leverages the best of both CPU and GPU processing capabilities while presenting a clean, unified interface to applications.