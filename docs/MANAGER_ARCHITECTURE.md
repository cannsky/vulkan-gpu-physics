# Manager-Based Physics Engine Architecture

## Overview

This refactoring implements a comprehensive manager-based architecture with singleton pattern for the Titanium GPU Physics engine. The new structure provides better organization, centralized resource management, and performance optimizations through a layer system.

## New Architecture

### Core Managers

#### BaseManager
- **Location**: `src/managers/BaseManager.h`
- **Purpose**: Base interface for all singleton managers
- **Features**: Common lifecycle methods (initialize, cleanup, isInitialized)

#### VulkanManager
- **Location**: `src/managers/vulkanmanager/`
- **Purpose**: Centralized Vulkan resource management
- **Features**: 
  - Singleton access to Vulkan instance, device, command pool
  - Memory management helpers
  - Buffer creation and copying utilities

#### PhysicsManager
- **Location**: `src/managers/physicsmanager/`
- **Purpose**: Replaces PhysicsEngine with manager pattern
- **Features**:
  - Coordinates all physics subsystems
  - Gravity management
  - Physics simulation orchestration

#### ParticleManager
- **Location**: `src/managers/particlemanager/`
- **Purpose**: Manages particle physics operations
- **Features**:
  - Particle creation and management
  - GPU operations for particles
  - Physics simulation for particles

#### CollisionManager
- **Location**: `src/managers/collisionmanager/`
- **Purpose**: Handles collision detection and resolution
- **Features**:
  - Collision detection
  - Contact resolution
  - GPU operations for collisions

#### ShaderManager
- **Location**: `src/managers/shadermanager/`
- **Purpose**: Shader compilation and management
- **Features**:
  - Shader caching system
  - SPIR-V loading
  - Common shader accessors

#### TestManager
- **Location**: `src/managers/testmanager/`
- **Purpose**: Enhanced test framework (moved from old location)
- **Features**:
  - Singleton pattern
  - BaseManager inheritance
  - Better test organization

### Advanced Features

#### LayerSystem
- **Location**: `src/managers/physicsmanager/LayerSystem.h`
- **Purpose**: Physics layer system for collision filtering
- **Features**:
  - Named layers (Default, Particles, Static, etc.)
  - Layer interaction matrix
  - Performance optimization through collision filtering
  - Layer mask operations

#### RigidBodyFactory
- **Location**: `src/managers/physicsmanager/rigidbodies/`
- **Purpose**: Factory pattern for creating rigid bodies
- **Features**:
  - Sphere, box, and plane creation
  - Automatic inertia calculation
  - Layer assignment
  - Material property management

## Usage Examples

### Initialize Managers
```cpp
// Initialize Vulkan system
auto& vulkanManager = VulkanManager::getInstance();
if (!vulkanManager.initialize()) {
    // Handle error
}

// Initialize physics system
auto& physicsManager = PhysicsManager::getInstance();
if (!physicsManager.initialize()) {
    // Handle error
}
```

### Create Layers
```cpp
auto& layerSystem = LayerSystem::getInstance();
auto particleLayer = layerSystem.createLayer("Particles");
auto staticLayer = layerSystem.createLayer("Static");

// Set layer interactions
layerSystem.setLayerInteraction(particleLayer, staticLayer, true);
```

### Create Rigid Bodies
```cpp
auto& rigidBodyFactory = RigidBodyFactory::getInstance();

// Create a sphere
auto sphere = rigidBodyFactory.createSphere(0.0f, 5.0f, 0.0f, 1.0f, 1.0f, particleLayer);

// Create a ground plane
auto ground = rigidBodyFactory.createStaticPlane(0.0f, staticLayer);
```

### Manage Particles
```cpp
auto& particleManager = ParticleManager::getInstance();

Particle particle = {};
// Set particle properties...
particleManager.addParticle(particle);
```

### Run Physics Simulation
```cpp
auto& physicsManager = PhysicsManager::getInstance();
physicsManager.setGravity(0.0f, -9.81f, 0.0f);

// In game loop
physicsManager.updatePhysics(deltaTime);
```

## Benefits

### 1. Better Structure
- Clear separation of concerns
- Each manager handles specific functionality
- Reduced coupling between systems

### 2. Singleton Pattern
- Global access to managers from anywhere
- Ensures single instance of critical systems
- Lazy initialization

### 3. Layer System
- Collision filtering for performance
- Flexible layer interactions
- Named layers for better organization

### 4. Factory Pattern
- Clean rigid body creation
- Automatic property calculation
- Consistent initialization

### 5. Centralized Resource Management
- VulkanManager handles all Vulkan operations
- Unified memory management
- Better resource cleanup

## Migration Guide

### Old vs New

#### Before (Old PhysicsEngine):
```cpp
auto vulkanContext = std::make_shared<VulkanContext>();
auto physicsEngine = std::make_unique<PhysicsEngine>(vulkanContext);
physicsEngine->initialize(1024);
physicsEngine->addParticle(particle);
```

#### After (New Manager System):
```cpp
auto& physicsManager = PhysicsManager::getInstance();
physicsManager.initialize();

auto& particleManager = ParticleManager::getInstance();
particleManager.addParticle(particle);
```

### Test System Migration

#### Before:
```cpp
#include "tests/framework/TestManager.h"
```

#### After:
```cpp
#include "managers/testmanager/TestManager.h"
```

## Performance Optimizations

### Layer System Benefits
- **Collision Filtering**: Objects on non-interacting layers skip collision detection
- **Broad Phase Optimization**: Early rejection based on layer masks
- **Memory Efficiency**: Reduced memory access for collision pairs

### Singleton Benefits
- **Reduced Object Creation**: Single instances reduce memory allocation
- **Cache Efficiency**: Consistent memory locations for frequently accessed objects
- **Resource Sharing**: Shared Vulkan resources across all systems

## Future Enhancements

The new architecture provides a foundation for:
- **Multi-threading**: Managers can be enhanced with thread-safe operations
- **Dynamic Loading**: Shaders and assets can be loaded on-demand
- **Profiling Integration**: Per-manager performance monitoring
- **Plugin System**: Additional managers can be easily integrated
- **Serialization**: Manager state can be saved/loaded for scene persistence

## Testing

All existing tests pass with the new architecture:
- 9/9 tests passing
- No regressions in functionality
- Enhanced test framework with better organization

The new manager-based architecture provides a solid foundation for future development while maintaining backward compatibility where possible.