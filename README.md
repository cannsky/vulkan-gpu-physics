# Titanium GPU Physics
[![Vulkan - Ubuntu](https://github.com/cannsky/vulkan-gpu-physics/actions/workflows/vulkan-ubuntu.yml/badge.svg)](https://github.com/cannsky/vulkan-gpu-physics/actions/workflows/vulkan-ubuntu.yml)

A unified GPU-accelerated physics simulation system using Vulkan compute shaders and C++. Supports both particle-based and rigid body physics with advanced collision detection.

## Features

- **Unified Physics Architecture**: Support for both particles and rigid bodies in a single system
- **GPU-accelerated physics**: Uses Vulkan compute shaders for high-performance physics calculations
- **Advanced Collision System**: Broad-phase and narrow-phase collision detection with multiple shape types
- **Rigid Body Dynamics**: Full 6-DOF rigid body simulation with proper mass properties
- **Particle Simulation**: Real-time particle physics with gravity, collisions, and dynamics
- **Configurable Logging**: Multi-category logging system with performance monitoring
- **Cross-platform**: Built with modern C++23 and Vulkan for cross-platform compatibility
- **Modular Design**: Clean separation between physics systems, collision detection, and Vulkan context

## Architecture

The system consists of several key components organized in a modular architecture:

### Core Systems

#### PhysicsEngine (`src/physics/`)
Main physics orchestrator that coordinates all subsystems:
- Manages particle system, rigid body system, and collision system
- Coordinates GPU compute operations and synchronization
- Provides unified interface for physics simulation

#### Particle System (`src/particles/`)
Manages particle-based physics simulation:
- GPU-accelerated particle integration
- Inter-particle collision detection
- Configurable gravity and forces

#### Rigid Body System (`src/rigidbody/`)
Handles rigid body dynamics:
- Support for spheres, boxes, and capsules
- Proper mass properties and inertia calculations
- Static and dynamic bodies

#### Collision System (`src/collision/`)
Advanced collision detection and response:
- Broad-phase spatial culling
- Narrow-phase shape-specific detection
- Contact resolution with restitution and friction

#### Logger System (`src/logger/`)
Configurable multi-category logging:
- Performance monitoring
- Physics event tracking
- Per-category filtering

#### Vulkan Context (`src/vulkan/`)
Vulkan abstraction layer:
- Device management and buffer allocation
- Compute pipeline creation and execution
- Cross-platform Vulkan initialization

## Building

### Prerequisites

- CMake 3.16 or higher
- C++23 compatible compiler
- Vulkan SDK or development libraries
- GLSL compiler (glslangValidator)

### Ubuntu/Debian Dependencies

```bash
sudo apt update
sudo apt install -y cmake build-essential
sudo apt install -y vulkan-tools libvulkan-dev vulkan-utility-libraries-dev
sudo apt install -y spirv-tools glslang-tools
```

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/cannsky/vulkan-gpu-physics.git
cd vulkan-gpu-physics

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)
```

## Running

```bash
# From the build directory
./titanium-gpu-physics
```

**Note**: The application requires a Vulkan-compatible GPU and drivers. On systems without Vulkan support, the application will fail to initialize the Vulkan context.

## Physics Simulation

The system demonstrates a basic particle physics simulation with:

- **100 particles** initialized with random positions and velocities
- **Gravity** applied in the negative Y direction (-9.81 m/s²)
- **Ground collision** at Y=0 with energy damping
- **Inter-particle collision** detection and response
- **Real-time statistics** showing FPS and particle height distribution

### Simulation Parameters

```cpp
// Adjustable in main.cpp
const int numParticles = 100;           // Number of particles
const float targetFrameTime = 1.0f/60.0f; // 60 FPS target
physicsSystem->setGravity(0.0f, -9.81f, 0.0f); // Earth gravity
```

### GPU Compute Shader

The physics calculations are performed entirely on the GPU using a compute shader (`src/shaders/particle_physics.comp`) that:

1. Updates particle velocities based on gravity
2. Updates particle positions based on velocities  
3. Handles ground plane collisions with energy damping
4. Performs basic inter-particle collision detection
5. Applies elastic collision response

## Extending the System

### Adding New Forces

Modify the compute shader to add new force types:

```glsl
// In particle_physics.comp
particles[index].velocity += windForce * ubo.deltaTime;
particles[index].velocity += magneticForce * ubo.deltaTime;
```

### Custom Particle Properties

Extend the `Particle` struct in `VulkanPhysics.h`:

```cpp
struct Particle {
    float position[3];
    float velocity[3];
    float mass;
    float radius;      // For variable-size particles
    float charge;      // For electromagnetic forces
    float temperature; // For thermal effects
};
```

### Performance Tuning

- Adjust workgroup size in the compute shader (currently 32 threads)
- Modify particle count and simulation frequency
- Implement spatial partitioning for collision detection
- Add multiple physics passes for complex interactions

## Technical Details

### Memory Management
- Uses host-visible buffers for CPU-GPU data transfer
- Implements proper Vulkan buffer creation and memory allocation
- Manages descriptor sets for shader resource binding

### Synchronization
- CPU waits for GPU completion each frame (simple approach)
- Could be optimized with double-buffering and fences
- Command buffer recording and submission

### Shader Compilation
- Shaders are compiled at build time using glslangValidator
- SPIR-V bytecode is embedded in the final executable
- Runtime shader loading from compiled .spv files

## License

This project is open source. See the repository for license details.

## Contributing

Contributions are welcome! Areas for improvement:

- More sophisticated collision detection algorithms
- Additional force types (springs, constraints, etc.)
- Performance optimizations and profiling
- Cross-platform testing and CI/CD
- Graphics rendering integration
