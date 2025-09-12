# Tulpar Physics

A GPU-based physics simulation system using Vulkan compute shaders and C++.

## Features

- **GPU-accelerated physics**: Uses Vulkan compute shaders for high-performance physics calculations
- **Particle simulation**: Supports real-time particle physics with gravity, collisions, and dynamics
- **Cross-platform**: Built with modern C++17 and Vulkan for cross-platform compatibility
- **Modular design**: Clean separation between Vulkan context management and physics system

## Architecture

The system consists of several key components:

### VulkanContext
Manages Vulkan initialization, device selection, and resource management:
- Instance creation with validation layers (debug builds)
- Physical device selection with compute queue support
- Logical device and command pool creation

### VulkanPhysics
Main physics system that orchestrates GPU-based physics calculations:
- Buffer management for particle data and uniforms
- Compute pipeline setup and shader loading
- Physics simulation loop with GPU synchronization

### Compute Shaders
GPU kernels written in GLSL that perform the actual physics calculations:
- Particle integration (position and velocity updates)
- Gravity application
- Basic collision detection and response
- Ground plane collision with damping

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler
- Vulkan SDK or development libraries
- GLSL compiler (glslangValidator or glslc)

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
git clone https://github.com/cannsky/tulpar-physics.git
cd tulpar-physics

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)
```

## Running

```bash
# From the build directory
./tulpar-physics
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