# Titanium Physics Engine

A hybrid GPU/CPU physics system that separates particle/fluid simulations (GPU) from complex rigidbody operations (CPU) using an ECS architecture.

## Architecture Overview

### Hybrid Design
- **GPU Physics System**: Handles particles and fluid simulations using Vulkan compute shaders
- **CPU Physics System**: Handles rigidbody dynamics with ECS (Entity Component System) architecture
- **Unified Interface**: Single `PhysicsEngine` class orchestrates both systems

### Directory Structure
```
src/
├── physics_engine.h/.cpp          # Main hybrid physics orchestrator
├── gpu_physics/                   # GPU-accelerated physics (particles/fluids)
│   └── GPUPhysicsSystem.h/.cpp
├── cpu_physics/                   # CPU-based physics (rigidbodies with ECS)
│   └── CPUPhysicsSystem.h/.cpp
└── ...
```

## Key Features

### CPU Physics System (ECS-based)
- **Components**: Transform, Physics, BoxCollider
- **Layer System**: Collision filtering with configurable layer interactions
- **Box Colliders Only**: Simplified to support only box-shaped colliders initially
- **AABB Collision Detection**: Efficient axis-aligned bounding box collision detection
- **Physics Integration**: Gravity, velocity, collision response with restitution and friction

### GPU Physics System (Vulkan-based)
- **Particle Simulation**: High-performance particle physics on GPU
- **Fluid Dynamics**: GPU-accelerated fluid simulation
- **Conditional Compilation**: Only compiles/runs when Vulkan is available

### Layer System
Physics layers enable collision filtering:
- Create named layers for different object types
- Configure which layers can interact with each other
- Efficient collision filtering during physics updates

## Usage Examples

### Basic Setup
```cpp
#include "physics_engine.h"

PhysicsEngine engine;
engine.initialize(1000, 100); // 1000 particles, 100 rigidbodies

// Create layers
uint32_t dynamicLayer = engine.createPhysicsLayer("Dynamic");
uint32_t staticLayer = engine.createPhysicsLayer("Static");

// Set layer interactions
engine.setLayerInteraction(dynamicLayer, staticLayer, true);
```

### Creating RigidBodies
```cpp
// Create a dynamic box
uint32_t boxId = engine.createRigidBody(
    0.0f, 5.0f, 0.0f,    // position (x, y, z)
    1.0f, 1.0f, 1.0f,    // size (width, height, depth)
    1.0f,                // mass
    dynamicLayer         // physics layer
);

// Create static ground
uint32_t groundId = engine.createRigidBody(
    0.0f, -1.0f, 0.0f,   // position
    10.0f, 0.2f, 10.0f,  // size
    0.0f,                // mass (0 = static)
    staticLayer          // layer
);
```

### Adding Particles (GPU Physics)
```cpp
// Add particle (only works if Vulkan is available)
engine.addParticle(
    0.0f, 10.0f, 0.0f,   // position
    0.0f, -1.0f, 0.0f,   // velocity
    1.0f                 // mass
);
```

### Physics Simulation
```cpp
float deltaTime = 1.0f / 60.0f; // 60 FPS
engine.updatePhysics(deltaTime);

// Access rigidbody data
auto* body = engine.getRigidBody(boxId);
if (body) {
    float x = body->transform.position[0];
    float y = body->transform.position[1];
    float z = body->transform.position[2];
}
```

## ECS Components

### RigidBodyComponent
Complete entity with all physics components:
```cpp
struct RigidBodyComponent {
    uint32_t entityId;
    TransformComponent transform;     // Position, rotation, scale
    PhysicsComponent physics;         // Velocity, mass, material properties
    BoxColliderComponent collider;    // Box dimensions
    uint32_t layer;                   // Physics layer
    bool hasCollider;                 // Enable/disable collision
};
```

### Component Details
- **TransformComponent**: Position (float[3]), rotation (quaternion), scale
- **PhysicsComponent**: Velocity, angular velocity, mass, restitution, friction, static flag
- **BoxColliderComponent**: Width, height, depth, enabled flag

## Collision System

### Layer-based Filtering
- Each rigidbody belongs to a physics layer
- Layers can be configured to interact or ignore each other
- Efficient pre-filtering before expensive collision detection

### Box-Box Collision Detection
- AABB (Axis-Aligned Bounding Box) collision detection
- Penetration depth calculation for all three axes
- Collision normal determination based on minimum penetration axis

### Collision Response
- Positional correction to separate overlapping bodies
- Impulse-based velocity changes
- Restitution (elasticity) and friction support
- Mass-proportional response for realistic physics

## Building and Testing

### CPU-Only Build (No Vulkan Required)
```bash
g++ -std=c++23 -I src \
    src/test_titanium_physics.cpp \
    src/physics_engine.cpp \
    src/cpu_physics/CPUPhysicsSystem.cpp \
    src/managers/logmanager/Logger.cpp \
    -o test_titanium_physics
```

### Full Build (With Vulkan)
```bash
# Requires Vulkan SDK
mkdir build && cd build
cmake .. -DVULKAN_AVAILABLE=ON
make
```

## Design Decisions

### Why Hybrid Architecture?
- **GPU Physics**: Optimal for thousands of simple particles/fluids
- **CPU Physics**: Better for complex rigidbody interactions, AI, gameplay logic
- **Separation of Concerns**: Each system optimized for its specific use case

### Why ECS for CPU Physics?
- **Performance**: Cache-friendly data layout
- **Flexibility**: Easy to add/remove components
- **Scalability**: Efficient iteration over entities
- **Maintainability**: Clear separation of data and behavior

### Why Box Colliders Only Initially?
- **Simplicity**: Easier to implement and debug
- **Performance**: AABB collision detection is very fast
- **Sufficient**: Many game objects can be approximated with boxes
- **Extensible**: Framework designed to easily add more collider types later

## Future Extensions

- Additional collider types (spheres, capsules, meshes)
- Joints and constraints system
- Advanced collision shapes (convex hulls)
- Multi-threaded CPU physics
- Integration with physics middleware (Box2D, Bullet)
- Visual debugging and profiling tools