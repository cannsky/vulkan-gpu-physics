# CPU Physics Engine

The CPU Physics Engine is a high-performance, ECS (Entity Component System) based physics system designed to handle complex rigidbody dynamics, collision detection, and physics simulation on the CPU.

## Overview

The CPU Physics Engine is part of the Titanium Physics hybrid architecture, focusing on complex rigidbody interactions that benefit from CPU processing. It implements a modern ECS architecture for optimal performance, flexibility, and maintainability.

## Key Features

### ECS Architecture
- **Data-Oriented Design**: Components are stored separately for cache-friendly access patterns
- **Entity Management**: Lightweight entity identifiers with component-based behavior
- **System-Based Processing**: Specialized systems handle different aspects of physics simulation
- **Scalable Performance**: Efficient iteration over entities with specific component combinations

### Physics Capabilities
- **Rigidbody Dynamics**: Full 6-DOF rigid body simulation with linear and angular motion
- **Box Colliders**: AABB collision detection and response (extensible to other shapes)
- **Gravity Simulation**: Configurable gravity system affecting physics bodies
- **Layer-Based Collision Filtering**: Physics layers for selective collision interactions
- **Mass and Inertia**: Realistic physics properties for all rigidbodies

### Performance Optimizations
- **Component Locality**: Components stored in typed containers for CPU cache efficiency
- **Batch Processing**: Systems process entities in batches for optimal performance
- **Memory Management**: Efficient allocation and deallocation of physics entities
- **Minimal Overhead**: Lightweight entity IDs and optimized component access

## Architecture

### Core Classes

#### CPUPhysicsEngine
The main orchestrator class that coordinates all CPU physics operations:

```cpp
namespace cpu_physics {
class CPUPhysicsEngine {
public:
    // Initialization and cleanup
    bool initialize(uint32_t maxRigidBodies = 512);
    void cleanup();
    
    // RigidBody management
    uint32_t createRigidBody(float x, float y, float z, 
                           float width, float height, float depth, 
                           float mass = 1.0f, uint32_t layer = 0);
    bool removeRigidBody(uint32_t entityId);
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
};
}
```

### ECS Integration

The CPU Physics Engine heavily emphasizes ECS usage through three main architectural components:

#### 1. ECS Manager
Manages entity lifecycle and component storage:
- **Entity Creation/Destruction**: Lightweight entity ID management
- **Component Storage**: Typed containers for each component type
- **Component Access**: Template-based component retrieval and management
- **Entity Queries**: Efficient queries for entities with specific component combinations

#### 2. Entity Factory
Handles creation of physics entities with all required components:
- **RigidBody Creation**: Combines Transform, Physics, and Collider components
- **Component Initialization**: Sets up realistic default values for physics properties
- **ECS Integration**: Seamlessly integrates with the ECS Manager
- **Batch Operations**: Efficient creation of multiple entities

#### 3. Systems Architecture
Specialized systems process specific aspects of physics:
- **Collision System**: Handles collision detection and response
- **Physics Integration**: Updates positions and velocities based on forces
- **Layer Management**: Manages collision filtering between physics layers

## ECS Components

### TransformComponent
Spatial representation of entities in 3D space:
```cpp
struct TransformComponent {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // quaternion (w, x, y, z)
    float scale[3] = {1.0f, 1.0f, 1.0f};
};
```

### PhysicsComponent
Dynamic properties for physics simulation:
```cpp
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
```

### BoxColliderComponent
Collision shape definition (currently only box colliders supported):
```cpp
struct BoxColliderComponent {
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    bool enabled = true;
};
```

## Physics Simulation Loop

The CPU Physics Engine follows a standard physics simulation pipeline:

1. **Force Application**: Apply gravity and other forces to physics bodies
2. **Integration**: Update velocities and positions using numerical integration
3. **Collision Detection**: Detect collisions between rigidbodies using AABB tests
4. **Collision Response**: Resolve collisions using impulse-based methods
5. **Constraint Solving**: Handle any additional constraints or joints
6. **State Updates**: Update final positions and orientations

## Layer System

The CPU Physics Engine implements a flexible layer system for collision filtering:

### Physics Layers
```cpp
struct PhysicsLayer {
    uint32_t id;
    std::string name;
    std::vector<uint32_t> interactionLayers;
};
```

### Layer Management
- **Layer Creation**: Create named physics layers for different object types
- **Interaction Setup**: Configure which layers can collide with each other
- **Runtime Filtering**: Efficient collision filtering during physics updates
- **Default Layer**: All objects start on layer 0 ("Default") which interacts with all layers

### Usage Examples
```cpp
// Create layers for different object types
uint32_t playerLayer = engine.createLayer("Player");
uint32_t enemyLayer = engine.createLayer("Enemy");
uint32_t environmentLayer = engine.createLayer("Environment");

// Configure interactions
engine.setLayerInteraction(playerLayer, enemyLayer, true);     // Player can collide with enemies
engine.setLayerInteraction(playerLayer, environmentLayer, true); // Player can collide with environment
engine.setLayerInteraction(enemyLayer, enemyLayer, false);     // Enemies don't collide with each other
```

## Performance Characteristics

### Memory Layout
- **Component Arrays**: Components stored in separate typed arrays for cache efficiency
- **Entity Lookup**: Fast entity to component mapping using hash tables
- **Memory Pools**: Efficient allocation strategies for frequently created/destroyed entities

### Computational Complexity
- **Collision Detection**: O(n²) broad phase with spatial optimization potential
- **Physics Integration**: O(n) linear complexity for position/velocity updates
- **ECS Queries**: O(n) iteration over entities with specific components

### Scalability
- **Entity Count**: Efficiently handles hundreds to thousands of rigidbodies
- **Component Access**: Constant-time component retrieval per entity
- **System Processing**: Linear scaling with entity count per system

## Integration with Hybrid System

The CPU Physics Engine is designed to work seamlessly with the GPU Physics Engine:

### Division of Responsibilities
- **CPU Physics**: Complex rigidbody dynamics, collision response, gameplay logic
- **GPU Physics**: Particle systems, fluid simulation, simple physics
- **Coordination**: Main PhysicsEngine class coordinates between both systems

### Data Exchange
- **Independent Operation**: CPU and GPU systems operate on separate entity sets
- **Synchronized Updates**: Both systems update during the same physics step
- **Shared Configuration**: Gravity and other global settings synchronized between systems

## Usage Examples

### Basic Setup
```cpp
#include "CPUPhysicsEngine.h"

cpu_physics::CPUPhysicsEngine engine;
engine.initialize(1000); // Support up to 1000 rigidbodies
engine.setGravity(0.0f, -9.81f, 0.0f); // Earth-like gravity
```

### Creating RigidBodies
```cpp
// Create a dynamic box at the origin
uint32_t boxId = engine.createRigidBody(
    0.0f, 10.0f, 0.0f,  // position (x, y, z)
    2.0f, 2.0f, 2.0f,   // size (width, height, depth)
    1.0f,               // mass
    0                   // layer (default)
);

// Create a static ground plane
uint32_t groundId = engine.createRigidBody(
    0.0f, -5.0f, 0.0f,   // position
    100.0f, 1.0f, 100.0f, // large flat box
    0.0f,                 // mass = 0 makes it static
    0                     // layer
);
```

### Physics Simulation
```cpp
// In your main loop
float deltaTime = 1.0f / 60.0f; // 60 FPS
engine.updatePhysics(deltaTime);

// Access rigidbody data (legacy compatibility)
RigidBodyComponent* box = engine.getRigidBody(boxId);
if (box) {
    float x = box->transform.position[0];
    float y = box->transform.position[1];
    float z = box->transform.position[2];
    // Use position for rendering, etc.
}
```

### Advanced ECS Usage
```cpp
// Direct ECS access for advanced users
auto ecsManager = engine.getECSManager();
auto collisionSystem = engine.getCollisionSystem();

// Query all entities with physics components
auto entities = ecsManager->getEntitiesWithComponent<PhysicsComponent>();
for (uint32_t entityId : entities) {
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    auto* transform = ecsManager->getComponent<TransformComponent>(entityId);
    
    // Custom physics processing
    if (physics && transform) {
        // Apply custom forces, modify properties, etc.
    }
}
```

## Design Benefits

### ECS Advantages
1. **Performance**: Cache-friendly data layout improves performance
2. **Flexibility**: Easy to add new component types and systems
3. **Maintainability**: Clear separation between data and behavior
4. **Scalability**: Systems can process thousands of entities efficiently
5. **Modularity**: Individual systems can be developed and tested independently

### Modern C++ Features
- **Smart Pointers**: Automatic memory management with shared_ptr
- **Templates**: Type-safe component access and storage
- **RAII**: Automatic resource cleanup in destructors
- **STL Containers**: Efficient data structures for component storage

## Future Extensions

The ECS architecture makes the CPU Physics Engine highly extensible:

### Additional Collider Types
- Sphere, capsule, and convex hull colliders
- Triangle mesh colliders for complex geometry
- Compound colliders for complex objects

### Advanced Physics Features
- Joint and constraint systems
- Soft body simulation
- Cloth and rope simulation
- Fluid-rigidbody interaction

### Performance Optimizations
- Multi-threaded physics processing
- Spatial acceleration structures (octrees, BVH)
- SIMD optimizations for batch operations
- Job system integration for parallel processing

### Enhanced ECS Features
- Component serialization/deserialization
- Runtime component type registration
- System dependency management
- Event-driven component updates