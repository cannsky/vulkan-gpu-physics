# INFO.md - AI Agent System Documentation

## System Overview

This is a GPU-accelerated physics simulation system built with Vulkan and C++. The system implements a unified physics approach supporting both particle-based and rigid body physics, with GPU-based collision detection and response.

## Architecture

### Directory Structure
```
src/
├── main.cpp                    # Entry point and simulation loop
├── physics/                    # Core physics engine and Vulkan context
│   ├── VulkanContext.h/cpp     # Vulkan initialization and device management
│   ├── PhysicsEngine.h/cpp     # Main physics orchestrator
│   ├── Particle.h              # Particle data structure
│   └── components/             # Physics subsystems
│       ├── BufferManager.h/cpp # GPU buffer management
│       └── ComputePipeline.h/cpp # Compute shader pipeline
├── particles/                  # Particle physics system
│   ├── ParticleSystem.h/cpp    # Particle simulation and management
├── rigidbody/                  # Rigid body physics system
│   ├── RigidBody.h             # Rigid body data structures
│   └── RigidBodySystem.h/cpp   # Rigid body simulation and management
├── collision/                  # Collision detection and response
│   ├── Contact.h               # Contact and collision pair structures
│   └── CollisionSystem.h/cpp   # Collision detection algorithms
├── logger/                     # Configurable logging system
│   └── Logger.h/cpp            # Multi-category logging with filtering
├── vulkan/                     # Vulkan abstraction layer
│   ├── VulkanInstance.h/cpp    # Vulkan instance management
│   ├── VulkanDevice.h/cpp      # Device selection and creation
│   └── VulkanCommandPool.h/cpp # Command buffer management
└── shaders/                    # GPU compute shaders
    └── particle_physics.comp   # Particle physics compute shader
```

## Core Systems

### 1. Physics Engine (`src/physics/PhysicsEngine.h/cpp`)
**Purpose**: Main orchestrator that coordinates all physics subsystems.

**Key Components**:
- Manages particle system, rigid body system, and collision system
- Coordinates GPU compute operations
- Handles physics timestep and simulation loop
- Provides unified interface for physics operations

**Key Methods**:
- `initialize(maxParticles)`: Initialize physics systems
- `addParticle(particle)`: Add particle to simulation
- `updatePhysics(deltaTime)`: Execute one physics timestep
- `setGravity(x, y, z)`: Set global gravity

### 2. Particle System (`src/particles/ParticleSystem.h/cpp`)
**Purpose**: Manages particle-based physics simulation.

**Data Structure** (`Particle`):
```cpp
struct Particle {
    float position[3];    // World position
    float velocity[3];    // Linear velocity
    float mass;          // Particle mass
    float padding;       // GPU alignment
};
```

**Key Methods**:
- `addParticle(particle)`: Add particle to system
- `updateUniformBuffer(deltaTime)`: Update shader uniforms
- `uploadParticlesToGPU()`: Transfer particle data to GPU
- `downloadParticlesFromGPU()`: Retrieve updated data from GPU

### 3. Rigid Body System (`src/rigidbody/RigidBodySystem.h/cpp`)
**Purpose**: Manages rigid body physics with shapes, mass properties, and dynamics.

**Data Structure** (`RigidBody`):
```cpp
struct RigidBody {
    float position[3];        // World position
    float velocity[3];        // Linear velocity
    float angularVelocity[3]; // Angular velocity
    float rotation[4];        // Quaternion orientation (w,x,y,z)
    float mass;              // Body mass
    float invMass;           // Inverse mass (0 for static)
    float inertia[3];        // Diagonal inertia tensor
    float invInertia[3];     // Inverse inertia tensor
    float restitution;       // Elasticity coefficient
    float friction;          // Friction coefficient
    uint32_t shapeType;      // Shape type (sphere, box, capsule)
    float shapeData[4];      // Shape parameters
    uint32_t isStatic;       // Static/dynamic flag
};
```

**Shape Types**:
- `SPHERE = 0`: Radius in `shapeData[0]`
- `BOX = 1`: Half-extents in `shapeData[0,1,2]`
- `CAPSULE = 2`: Radius and half-height

**Key Methods**:
- `createSphere/Box/StaticPlane()`: Create common shapes
- `createRigidBody(body)`: Add custom rigid body
- `updateUniformBuffer(deltaTime)`: Update physics parameters

### 4. Collision System (`src/collision/CollisionSystem.h/cpp`)
**Purpose**: Broad-phase and narrow-phase collision detection with contact resolution.

**Data Structures**:
```cpp
struct Contact {
    float position[3];    // Contact point
    float normal[3];      // Contact normal
    float penetration;    // Penetration depth
    uint32_t bodyIdA;     // First body ID
    uint32_t bodyIdB;     // Second body ID
    float restitution;    // Combined restitution
    float friction;       // Combined friction
};

struct CollisionPair {
    uint32_t bodyIdA;     // First body ID
    uint32_t bodyIdB;     // Second body ID
    uint32_t isActive;    // Collision status
};
```

**Collision Detection Pipeline**:
1. **Broad Phase**: AABB-based spatial culling
2. **Narrow Phase**: Shape-specific collision tests
   - Sphere vs Sphere
   - Sphere vs Box  
   - Box vs Box (simplified AABB)
3. **Contact Resolution**: Impulse-based response

**Key Methods**:
- `updateBroadPhase()`: Generate collision pairs
- `detectCollisions()`: Perform narrow-phase detection
- `resolveContacts()`: Apply collision response

### 5. Logger System (`src/logger/Logger.h/cpp`)
**Purpose**: Configurable multi-category logging with filtering.

**Log Categories**:
- `GENERAL`: General system messages
- `PHYSICS`: Physics engine events
- `COLLISION`: Collision detection/response
- `RIGIDBODY`: Rigid body operations
- `PARTICLES`: Particle system events
- `VULKAN`: Vulkan API operations
- `PERFORMANCE`: Performance metrics

**Log Levels**:
- `TRACE`: Detailed debugging information
- `DEBUG`: Debug information
- `INFO`: General information
- `WARN`: Warning messages
- `ERROR`: Error messages

**Configuration**:
- Per-category enable/disable
- Minimum log level filtering
- Console and file output
- Timestamp formatting

**Usage Examples**:
```cpp
Logger::getInstance().setLogLevel(LogLevel::DEBUG);
Logger::getInstance().enableCategory(LogCategory::COLLISION);
Logger::getInstance().setOutputFile("physics.log");

LOG_COLLISION_INFO("Detected collision between bodies 1 and 2");
Logger::getInstance().logFrameTime(0.016f);
```

### 6. Vulkan Context (`src/physics/VulkanContext.h/cpp`)
**Purpose**: Vulkan initialization, device management, and resource creation.

**Key Components**:
- Instance creation with validation layers
- Physical device selection (compute queue support)
- Logical device and command pool creation
- Memory allocation and buffer management

## GPU Compute Pipeline

### Current Shader (`src/shaders/particle_physics.comp`)
**Purpose**: GPU kernel for particle physics simulation.

**Operations**:
1. Apply gravity to particle velocities
2. Integrate position from velocity
3. Handle ground plane collision with damping
4. Perform inter-particle collision detection
5. Apply elastic collision response

**Workgroup Size**: 32 threads (configurable)

**Uniform Buffer**:
```glsl
layout(binding = 1) uniform UniformBufferObject {
    float deltaTime;
    vec3 gravity;
    uint particleCount;
} ubo;
```

### Future Shader Extensions
The system is designed to support additional compute shaders for:
- Rigid body integration
- Advanced collision detection
- Constraint solving
- Spatial partitioning

## Build System

### CMake Configuration (`CMakeLists.txt`)
- C++23 standard
- Vulkan SDK dependency
- Automatic shader compilation (GLSL → SPIR-V)
- Cross-platform support

### Dependencies
- **Vulkan SDK**: Graphics API for compute shaders
- **glslangValidator**: Shader compilation tool
- **CMake 3.16+**: Build system
- **C++23 compiler**: Language support

### Build Process
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Performance Characteristics

### Scalability
- **Particles**: Tested up to 1024 particles
- **Rigid Bodies**: Designed for 512 bodies
- **Contacts**: Support for 1024 simultaneous contacts
- **GPU Workgroups**: 32 threads per workgroup (optimal for most GPUs)

### Memory Layout
- Structure-of-Arrays (SoA) for GPU efficiency
- 16-byte aligned data structures
- Host-visible buffers for CPU-GPU transfer
- Dedicated GPU memory for compute operations

### Synchronization
- CPU-GPU synchronization per frame (simple)
- Command buffer recording and submission
- Fence-based completion detection

## Extension Points

### Adding New Physics Systems
1. Create new directory under `src/`
2. Implement system class with standard interface
3. Add to `PhysicsEngine` orchestration
4. Update CMakeLists.txt
5. Add logging categories as needed

### Adding New Collision Shapes
1. Add shape type to `RigidBodyShape` enum
2. Implement collision detection functions
3. Add inertia calculation methods
4. Update GPU shaders if needed

### GPU Shader Development
1. Create `.comp` files in `src/shaders/`
2. Follow existing uniform buffer layouts
3. Use workgroup size of 32 for compatibility
4. Handle boundary conditions (thread count vs data count)

## Debugging and Profiling

### Logging Configuration
```cpp
// Enable detailed collision logging
Logger::getInstance().setLogLevel(LogLevel::DEBUG);
Logger::getInstance().enableCategory(LogCategory::COLLISION);

// Performance monitoring
Logger::getInstance().logFrameTime(deltaTime);
Logger::getInstance().logCollisionCount(contactCount);
```

### Common Issues
1. **Vulkan Initialization**: Check driver support and validation layers
2. **Shader Compilation**: Verify glslangValidator installation
3. **Memory Allocation**: Monitor GPU memory usage
4. **Performance**: Profile GPU compute times vs CPU overhead

### Validation
- Vulkan validation layers in debug builds
- CPU-side collision detection for verification
- Physics constraint violation detection
- Energy conservation monitoring

## Implementation Status

### Completed
- ✅ Particle system with GPU physics
- ✅ Rigid body system architecture
- ✅ Basic collision detection (sphere, box)
- ✅ Configurable logging system
- ✅ Vulkan compute pipeline
- ✅ Build system and documentation

### In Progress / Future Work
- 🔄 GPU-based rigid body integration
- 🔄 Advanced collision response
- 🔄 Spatial partitioning for broad phase
- 🔄 Constraint solver
- 🔄 Performance optimization
- 🔄 Comprehensive testing framework

## Code Conventions

### Naming
- Classes: PascalCase (`PhysicsEngine`)
- Methods: camelCase (`updatePhysics`)
- Variables: camelCase (`deltaTime`)
- Constants: UPPER_CASE (`MAX_PARTICLES`)
- Files: PascalCase matching class names

### Memory Management
- RAII principles throughout
- Smart pointers for ownership
- Explicit cleanup methods
- Vulkan object lifetime management

### Error Handling
- Return codes for critical operations
- Logging for error conditions
- Graceful degradation where possible
- Vulkan validation layer integration