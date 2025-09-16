# GPU Physics Engine

The GPU Physics Engine is a high-performance Vulkan-based physics system designed to handle massive particle simulations and fluid dynamics using GPU compute shaders.

## Overview

The GPU Physics Engine is part of the Titanium Physics hybrid architecture, focusing on massively parallel physics simulations that benefit from GPU processing. It leverages Vulkan compute shaders to simulate thousands of particles efficiently, handling scenarios where CPU-based physics would be impractical.

## Key Features

### GPU-Accelerated Physics
- **Compute Shader Pipeline**: Custom Vulkan compute shaders for physics calculations
- **Massively Parallel**: Efficient processing of thousands of particles simultaneously
- **High Performance**: GPU acceleration provides significant speedup over CPU alternatives
- **Low Latency**: Direct GPU memory operations minimize CPU-GPU transfer overhead

### Particle System Capabilities
- **Large Scale Simulations**: Handle 1000+ particles with real-time performance
- **Flexible Particle Properties**: Position, velocity, mass, and custom attributes
- **Physics Integration**: Accurate numerical integration using compute shaders
- **Collision Response**: GPU-based particle collision detection and response

### Vulkan Integration
- **Modern Graphics API**: Uses Vulkan for maximum performance and control
- **Compute Pipeline**: Dedicated compute shader pipeline for physics calculations
- **Buffer Management**: Efficient GPU buffer allocation and management
- **Command Buffer Recording**: Optimized command buffer usage for compute operations

## Architecture

### Core Classes

#### GPUPhysicsEngine
The main orchestrator class for GPU-based physics operations:

```cpp
namespace gpu_physics {
class GPUPhysicsEngine {
public:
    GPUPhysicsEngine(std::shared_ptr<VulkanContext> context);
    
    // Initialization and cleanup
    bool initialize(uint32_t maxParticles = 1024);
    void cleanup();
    
    // Particle management
    bool addParticle(const Particle& particle);
    bool addParticle(float x, float y, float z, float vx = 0.0f, float vy = 0.0f, float vz = 0.0f, float mass = 1.0f);
    std::vector<Particle> getParticles() const;
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // GPU buffer management
    void uploadParticlesToGPU();
    void downloadParticlesFromGPU();
};
}
```

### GPU Pipeline Architecture

The GPU Physics Engine implements a sophisticated pipeline for physics computation:

#### 1. Vulkan Context Management
- **Instance Creation**: Initialize Vulkan instance with required extensions
- **Device Selection**: Choose appropriate GPU with compute shader support
- **Queue Management**: Set up compute queues for physics operations
- **Memory Allocation**: Efficient GPU memory management for physics data

#### 2. Compute Shader Pipeline
- **Shader Compilation**: Compile SPIR-V compute shaders for physics calculations
- **Pipeline Creation**: Set up compute pipeline with appropriate bindings
- **Descriptor Sets**: Manage uniform buffers and storage buffers
- **Dispatch Operations**: Execute compute shaders with optimal workgroup sizes

#### 3. Buffer Management
- **Storage Buffers**: GPU buffers for particle data (position, velocity, etc.)
- **Uniform Buffers**: Simulation parameters (gravity, time step, boundaries)
- **Staging Buffers**: Temporary buffers for CPU-GPU data transfer
- **Memory Synchronization**: Proper barriers and synchronization for data consistency

## Particle System

### Particle Structure
The fundamental data structure for GPU physics simulation:

```cpp
struct Particle {
    float position[3];    // World space position (x, y, z)
    float velocity[3];    // Velocity vector (vx, vy, vz)
    float mass;          // Particle mass for physics calculations
    float padding;       // GPU alignment padding
};
```

**GPU Alignment**: Structure is designed for optimal GPU memory layout with proper alignment for compute shader access.

### Particle Management

#### Adding Particles
```cpp
// Method 1: Direct particle object
Particle particle;
particle.position[0] = 0.0f; particle.position[1] = 10.0f; particle.position[2] = 0.0f;
particle.velocity[0] = 1.0f; particle.velocity[1] = 0.0f; particle.velocity[2] = 0.0f;
particle.mass = 1.0f;
engine.addParticle(particle);

// Method 2: Convenience function
engine.addParticle(
    0.0f, 10.0f, 0.0f,  // position
    1.0f, 0.0f, 0.0f,   // velocity
    1.0f                // mass
);
```

#### Particle Limits
- **Maximum Count**: Configurable at initialization (default: 1024)
- **Dynamic Allocation**: Efficient addition of particles up to maximum
- **Memory Management**: Automatic GPU memory allocation for particle storage

### Physics Simulation

#### Gravity System
```cpp
struct GravityUniform {
    float x = 0.0f;
    float y = -9.81f;   // Earth-like gravity
    float z = 0.0f;
    float padding;      // GPU alignment
};
```

#### Integration Methods
The GPU Physics Engine uses numerical integration methods implemented in compute shaders:

1. **Explicit Euler Integration**: Simple and fast integration method
2. **Force Application**: Gravity and other forces applied per particle
3. **Velocity Updates**: Update particle velocities based on applied forces
4. **Position Updates**: Update particle positions based on velocities

#### Simulation Loop
```cpp
void GPUPhysicsEngine::updatePhysics(float deltaTime) {
    // Upload current state to GPU
    uploadParticlesToGPU();
    
    // Record compute commands
    recordComputeCommandBuffer();
    
    // Submit compute work to GPU
    submitComputeCommands();
    
    // Download results from GPU
    downloadParticlesFromGPU();
}
```

## Vulkan Components

### VulkanContext
**Purpose**: Manages Vulkan instance, device, and basic resources.

**Responsibilities**:
- Vulkan instance creation with validation layers
- Physical device selection based on compute capabilities
- Logical device creation with required queues
- Basic resource management (command pools, etc.)

### BufferManager
**Purpose**: Handles GPU buffer allocation and management.

**Key Features**:
- **Storage Buffers**: For particle data (positions, velocities, masses)
- **Uniform Buffers**: For simulation parameters (gravity, time step)
- **Staging Buffers**: For CPU-GPU data transfers
- **Memory Types**: Appropriate memory type selection for different use cases

```cpp
class BufferManager {
public:
    // Buffer creation
    VkBuffer createStorageBuffer(VkDeviceSize size);
    VkBuffer createUniformBuffer(VkDeviceSize size);
    VkBuffer createStagingBuffer(VkDeviceSize size);
    
    // Memory management
    void allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);
    void* mapBufferMemory(VkBuffer buffer);
    void unmapBufferMemory(VkBuffer buffer);
    
    // Data transfer
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
```

### ComputePipeline
**Purpose**: Manages compute shader pipeline for physics calculations.

**Components**:
- **Shader Module**: Compiled SPIR-V compute shader
- **Pipeline Layout**: Descriptor set layouts for buffers and uniforms
- **Compute Pipeline**: Complete pipeline object for dispatch operations
- **Descriptor Sets**: Bound resources for shader access

```cpp
class ComputePipeline {
public:
    // Pipeline creation
    bool createComputePipeline(const std::string& shaderPath);
    void createDescriptorSets(VkBuffer particleBuffer, VkBuffer uniformBuffer);
    
    // Compute operations
    void bindPipeline(VkCommandBuffer commandBuffer);
    void bindDescriptorSets(VkCommandBuffer commandBuffer);
    void dispatch(VkCommandBuffer commandBuffer, uint32_t particleCount);
};
```

## Compute Shader Implementation

### Shader Structure
The physics compute shader implements the core simulation logic:

```glsl
#version 450

// Workgroup size for optimal GPU utilization
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

// Particle data structure
struct Particle {
    vec3 position;
    float mass;
    vec3 velocity;
    float padding;
};

// Storage buffers
layout(std140, binding = 0) restrict buffer ParticleBuffer {
    Particle particles[];
};

// Uniform buffer for simulation parameters
layout(binding = 1) uniform SimulationParams {
    vec3 gravity;
    float deltaTime;
    uint particleCount;
};

void main() {
    uint index = gl_GlobalInvocationID.x;
    
    // Boundary check
    if (index >= particleCount) return;
    
    // Apply gravity
    particles[index].velocity += gravity * deltaTime;
    
    // Update position
    particles[index].position += particles[index].velocity * deltaTime;
    
    // Simple boundary conditions
    if (particles[index].position.y < -10.0) {
        particles[index].position.y = -10.0;
        particles[index].velocity.y = -particles[index].velocity.y * 0.8; // Damping
    }
}
```

### Shader Features
- **Workgroup Size**: Optimized for GPU architecture (typically 32 threads)
- **Boundary Checking**: Prevents out-of-bounds memory access
- **Physics Integration**: Explicit Euler integration for simplicity and speed
- **Boundary Conditions**: Simple collision response with ground plane

### Performance Optimization
- **Memory Coalescing**: Contiguous memory access patterns for optimal GPU performance
- **Branch Divergence**: Minimal branching to avoid GPU thread divergence
- **Workgroup Efficiency**: Workgroup size chosen for optimal occupancy

## GPU Memory Management

### Buffer Types

#### Storage Buffers
```cpp
// Particle data storage
VkBuffer particleBuffer;
VkDeviceMemory particleBufferMemory;

// Buffer creation for GPU-local storage
VkBufferCreateInfo bufferInfo = {};
bufferInfo.size = maxParticles * sizeof(Particle);
bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
```

#### Uniform Buffers
```cpp
// Simulation parameters
struct SimulationParams {
    float gravity[3];
    float deltaTime;
    uint32_t particleCount;
    float padding[3]; // GPU alignment
};

VkBuffer uniformBuffer;
VkDeviceMemory uniformBufferMemory;
```

#### Staging Buffers
```cpp
// For CPU-GPU data transfer
VkBuffer stagingBuffer;
VkDeviceMemory stagingBufferMemory;

// Host-visible memory for CPU access
VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
```

### Memory Synchronization

#### Pipeline Barriers
```cpp
// Ensure compute shader writes complete before next dispatch
VkMemoryBarrier memoryBarrier = {};
memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

vkCmdPipelineBarrier(
    commandBuffer,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0, 1, &memoryBarrier, 0, nullptr, 0, nullptr
);
```

#### Buffer Barriers
```cpp
// Synchronize buffer access between compute and transfer operations
VkBufferMemoryBarrier bufferBarrier = {};
bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
bufferBarrier.buffer = particleBuffer;
bufferBarrier.offset = 0;
bufferBarrier.size = VK_WHOLE_SIZE;
```

## Performance Characteristics

### GPU Utilization
- **Parallel Processing**: Each particle processed by separate GPU thread
- **Occupancy**: Workgroup size optimized for GPU architecture
- **Memory Bandwidth**: Efficient use of GPU memory bandwidth
- **Compute Units**: Full utilization of available compute units

### Scaling Characteristics
- **Linear Scaling**: Performance scales linearly with particle count (up to GPU limits)
- **Memory Bound**: Performance typically limited by memory bandwidth, not compute
- **Batch Size**: Larger batches improve GPU utilization efficiency

### Performance Metrics
```cpp
// Typical performance characteristics (hardware dependent)
// - 1,000 particles: 60+ FPS
// - 10,000 particles: 30+ FPS  
// - 100,000 particles: Depends on GPU memory and compute capability
```

## Integration with Hybrid System

### CPU-GPU Coordination
The GPU Physics Engine operates independently but coordinates with CPU physics:

```cpp
// Main physics update coordinates both systems
void PhysicsEngine::updatePhysics(float deltaTime) {
    // Update GPU particles
    if (gpuPhysics) {
        gpuPhysics->updatePhysics(deltaTime);
    }
    
    // Update CPU rigidbodies
    if (cpuPhysics) {
        cpuPhysics->updatePhysics(deltaTime);
    }
}
```

### Shared Configuration
```cpp
// Synchronized gravity settings
void PhysicsEngine::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    if (gpuPhysics) {
        gpuPhysics->setGravity(x, y, z);
    }
    if (cpuPhysics) {
        cpuPhysics->setGravity(x, y, z);
    }
}
```

### Data Isolation
- **Separate Entity Sets**: GPU and CPU physics operate on different entity types
- **Independent Memory**: GPU particles stored in GPU memory, CPU rigidbodies in system memory
- **No Direct Interaction**: Particles and rigidbodies don't directly interact (can be extended)

## Usage Examples

### Basic Setup
```cpp
#include "GPUPhysicsEngine.h"

// Create Vulkan context
auto vulkanContext = std::make_shared<VulkanContext>();
vulkanContext->initialize();

// Create GPU physics engine
gpu_physics::GPUPhysicsEngine engine(vulkanContext);
engine.initialize(10000); // Support up to 10,000 particles
engine.setGravity(0.0f, -9.81f, 0.0f);
```

### Adding Particles
```cpp
// Add particles in a grid pattern
for (int x = 0; x < 10; ++x) {
    for (int y = 0; y < 10; ++y) {
        for (int z = 0; z < 10; ++z) {
            engine.addParticle(
                x * 2.0f, y * 2.0f + 20.0f, z * 2.0f,  // position
                0.0f, 0.0f, 0.0f,                        // initial velocity
                1.0f                                     // mass
            );
        }
    }
}
```

### Simulation Loop
```cpp
// In your main loop
float deltaTime = 1.0f / 60.0f; // 60 FPS
engine.updatePhysics(deltaTime);

// Get particle data for rendering
auto particles = engine.getParticles();
for (const auto& particle : particles) {
    // Render particle at particle.position
    renderParticle(particle.position[0], particle.position[1], particle.position[2]);
}
```

### Advanced GPU Access
```cpp
// Access GPU components for advanced usage
auto bufferManager = engine.getBufferManager();
auto computePipeline = engine.getComputePipeline();

// Custom GPU operations (advanced users)
// - Additional compute shaders
// - Custom buffer management
// - Performance profiling
```

## Future Extensions

### Enhanced Particle Features
- **Particle-Particle Collision**: Inter-particle collision detection and response
- **Fluid Simulation**: SPH (Smoothed Particle Hydrodynamics) fluid simulation
- **Particle Rendering**: Direct GPU particle rendering without CPU readback
- **Multiple Particle Types**: Different particle types with different properties

### Advanced Physics
- **Soft Body Physics**: Particle-based soft body simulation
- **Cloth Simulation**: Particle-based cloth dynamics
- **Constraint Systems**: Distance constraints, angular constraints
- **Force Fields**: Custom force field applications

### Performance Optimizations
- **Spatial Acceleration**: GPU-based spatial data structures (uniform grids, octrees)
- **Asynchronous Compute**: Overlap compute with other GPU work
- **Multi-GPU Support**: Distribution across multiple GPUs
- **Compute/Graphics Interop**: Direct integration with rendering pipeline

### Vulkan Features
- **Timeline Semaphores**: Advanced synchronization for complex pipelines
- **Ray Tracing**: Hardware-accelerated collision detection (RTX GPUs)
- **Subgroup Operations**: GPU architecture-specific optimizations
- **Mesh Shaders**: Integration with modern geometry pipeline

The GPU Physics Engine provides a solid foundation for high-performance particle physics simulation, with a clean architecture that supports future enhancements and optimizations.