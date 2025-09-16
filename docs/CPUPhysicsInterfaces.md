# CPU Physics Engine Interfaces

This document describes the abstract interfaces added to the CPU Physics Engine for improved modularity and extensibility.

## Overview

The CPU Physics Engine now includes abstract interfaces that provide a foundation for polymorphic behavior and allow different implementations to be handled uniformly. These interfaces are located in `CPUPhysicsEngine/interfaces/`.

## Architecture

The interface system consists of three main abstract classes:

### 1. CPUPhysicsComponent
**Location**: `interfaces/CPUPhysicsComponent.h`

Abstract base class for all CPU physics components. Provides:
- Component type identification (Transform, Physics, BoxCollider, etc.)
- Enable/disable functionality
- Validation and reset methods
- Deep copying (clone) support
- Memory footprint information

**Component Types**:
- `TRANSFORM`: Position, rotation, scale
- `PHYSICS`: Mass, velocity, material properties
- `BOX_COLLIDER`: Box-shaped collision geometry
- `SPHERE_COLLIDER`: Sphere-shaped collision geometry (future)
- `CUSTOM`: Starting point for custom component types

### 2. CPUPhysicsEntity
**Location**: `interfaces/CPUPhysicsEntity.h`

Abstract base class for physics entities. An entity is a container for components and represents a single physics object. Provides:
- Component management (add, remove, get)
- Entity activation/deactivation
- Physics layer assignment
- User data attachment
- Entity cloning
- Validation

### 3. CPUPhysicsSystem
**Location**: `interfaces/CPUPhysicsSystem.h`

Abstract base class for physics systems. A system processes entities with specific component combinations. Provides:
- System lifecycle management (initialize, cleanup)
- Entity processing with filtering
- Priority-based execution ordering
- Performance monitoring
- Component requirements specification

**System Types**:
- `COLLISION`: Collision detection and response
- `INTEGRATION`: Physics integration (forces, velocities)
- `RENDERING`: Rendering-related operations
- `CONSTRAINT`: Constraint solving
- `PARTICLE`: Particle system processing
- `CUSTOM`: Starting point for custom system types

**Priority Levels**:
- `HIGHEST` (0): Critical systems that run first
- `HIGH` (100): Important systems like collision
- `NORMAL` (500): Standard systems
- `LOW` (900): Less critical systems
- `LOWEST` (999): Systems that run last

## Concrete Implementations

### Concrete Components
**Location**: `concrete/ConcreteComponents.h`

Wrapper classes that implement the interfaces while wrapping existing ECS components:
- `ConcreteTransformComponent`: Wraps `TransformComponent`
- `ConcretePhysicsComponent`: Wraps `PhysicsComponent`
- `ConcreteBoxColliderComponent`: Wraps `BoxColliderComponent`

### Concrete Entity
**Location**: `concrete/ConcreteEntity.h`

Complete implementation of `CPUPhysicsEntity` interface:
- `ConcreteEntity`: Component storage using hash map
- `ConcreteEntityFactory`: Factory for common entity configurations

### Base System
**Location**: `systems/BaseCPUPhysicsSystem.h`

Base implementation providing common ECS functionality:
- `BaseCPUPhysicsSystem`: ECS integration and performance tracking

### Enhanced Collision System
**Location**: `systems/EnhancedCPUPhysicsCollisionSystem.h`

Interface-aware collision system:
- `EnhancedCPUPhysicsCollisionSystem`: Implements interface while maintaining ECS compatibility

## Usage Examples

### Creating Entities with Interfaces

```cpp
#include "interfaces/interfaces.h"
#include "concrete/ConcreteEntity.h"

// Create a rigidbody entity
auto entity = cpu_physics::concrete::ConcreteEntityFactory::createRigidBodyEntity(
    1,                          // entity ID
    0.0f, 5.0f, 0.0f,          // position (x, y, z)
    2.0f, 1.0f, 2.0f,          // dimensions (width, height, depth)
    1.5f,                       // mass
    0                           // physics layer
);

// Check components
bool hasPhysics = entity->hasComponent(
    cpu_physics::interfaces::CPUPhysicsComponent::ComponentType::PHYSICS
);

// Set physics layer
entity->setPhysicsLayer(1);
```

### Using Enhanced Systems

```cpp
#include "systems/EnhancedCPUPhysicsCollisionSystem.h"

// Create enhanced collision system
auto ecsManager = std::make_shared<cpu_physics::ECSManager>();
auto collisionSystem = std::make_unique<cpu_physics::EnhancedCPUPhysicsCollisionSystem>(ecsManager);

// Initialize and configure
collisionSystem->initialize();
collisionSystem->setGravity(0.0f, -9.81f, 0.0f);

// Process entities
std::vector<cpu_physics::interfaces::CPUPhysicsEntity*> entities = {entity.get()};
collisionSystem->update(entities, 0.016f); // 16ms timestep
```

### Component Creation and Validation

```cpp
// Create components using concrete implementations
cpu_physics::TransformComponent transform;
transform.position[0] = 1.0f;
transform.position[1] = 2.0f;
transform.position[2] = 3.0f;

auto concreteTransform = std::make_unique<cpu_physics::concrete::ConcreteTransformComponent>(transform);

// Validate component
if (concreteTransform->validate()) {
    std::cout << "Component is valid: " << concreteTransform->getTypeName() << std::endl;
}

// Add to entity
entity->addComponent(std::move(concreteTransform));
```

## Benefits

1. **Modularity**: Clear separation of concerns with well-defined interfaces
2. **Extensibility**: Easy to add new component types and systems
3. **Polymorphism**: Uniform handling of different component/system types
4. **Testing**: Interfaces enable easier mocking and unit testing
5. **Backward Compatibility**: Existing ECS code continues to work unchanged
6. **Performance**: Interface overhead is minimal due to virtual function inlining

## Integration with Existing Code

The interfaces are designed to work alongside the existing ECS architecture:

- **Existing systems** continue to work with ECS components directly
- **Enhanced systems** can work with both interfaces and ECS components
- **Factories** can create both ECS entities and interface-based entities
- **Backward compatibility** is maintained for all existing functionality

## Version Information

- **Interface Version**: 1.0.0
- **Location**: `interfaces/interfaces.h`
- **Utility Functions**: Component/system type string conversion helpers

For a complete demonstration of the interfaces, see `tests/interface_demo.cpp` which shows all major functionality in action.