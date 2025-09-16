# CPU Engine Components

This document provides detailed information about all components, managers, factories, and systems that make up the CPU Physics Engine's ECS architecture.

## Table of Contents

1. [Components](#components)
2. [Managers](#managers)
3. [Factories](#factories)
4. [Systems](#systems)
5. [Integration Architecture](#integration-architecture)

## Components

Components are pure data structures that define the properties and state of entities in the physics system.

### TransformComponent

**Purpose**: Spatial representation of entities in 3D space.

**Location**: `src/physics_engine/cpu_physics_engine/components.h`

```cpp
struct TransformComponent {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // quaternion (w, x, y, z)
    float scale[3] = {1.0f, 1.0f, 1.0f};
};
```

**Fields**:
- `position[3]`: World space position (x, y, z)
- `rotation[4]`: Quaternion rotation (w, x, y, z) for arbitrary 3D rotations
- `scale[3]`: Local scale factors (x, y, z)

**Usage**:
- Updated by physics systems during simulation
- Used by rendering systems for visual representation
- Provides spatial queries and transformations

### PhysicsComponent

**Purpose**: Dynamic properties for physics simulation and behavior.

**Location**: `src/physics_engine/cpu_physics_engine/components.h`

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

**Fields**:
- `velocity[3]`: Linear velocity in world space (m/s)
- `angularVelocity[3]`: Angular velocity around world axes (rad/s)
- `mass`: Object mass in kilograms
- `invMass`: Inverse mass (1/mass) for performance; 0 for static/infinite mass
- `restitution`: Coefficient of restitution (0 = inelastic, 1 = perfectly elastic)
- `friction`: Coefficient of friction for surface interactions
- `isStatic`: Whether the object is static (immovable)
- `useGravity`: Whether gravity affects this object

**Physics Properties**:
- Static objects (mass = 0, invMass = 0) don't move but participate in collisions
- Dynamic objects respond to forces and collisions
- Restitution controls bounciness in collisions
- Friction affects sliding behavior on surfaces

### BoxColliderComponent

**Purpose**: Defines collision shape and boundaries for physics objects.

**Location**: `src/physics_engine/cpu_physics_engine/components.h`

```cpp
struct BoxColliderComponent {
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    bool enabled = true;
};
```

**Fields**:
- `width`: Box extent along X-axis
- `height`: Box extent along Y-axis  
- `depth`: Box extent along Z-axis
- `enabled`: Whether collision detection is active

**Collision Detection**:
- Uses Axis-Aligned Bounding Box (AABB) algorithm
- Fast and efficient for box-shaped objects
- Suitable for many game objects (crates, buildings, platforms)
- Foundation for more complex collision shapes

### RigidBodyComponent (Legacy)

**Purpose**: Complete rigidbody entity with all components for legacy compatibility.

**Location**: `src/physics_engine/cpu_physics_engine/components.h`

```cpp
struct RigidBodyComponent {
    uint32_t entityId;
    TransformComponent transform;
    PhysicsComponent physics;
    BoxColliderComponent collider;
    uint32_t layer = 0; // Physics layer for collision filtering
    bool hasCollider = true;
};
```

**Purpose**:
- Provides legacy API compatibility
- Aggregates all components for a complete rigidbody
- Used by legacy code that expects monolithic rigidbody objects
- Bridges between ECS architecture and traditional physics APIs

## Managers

Managers handle the lifecycle and organization of entities and components within the ECS framework.

### ECS Manager

**Purpose**: Core ECS implementation managing entities and components.

**Location**: `src/physics_engine/cpu_physics_engine/managers/ecs_manager/`

#### Key Responsibilities:
1. **Entity Management**: Create, destroy, and validate entities
2. **Component Storage**: Type-safe storage for all component types
3. **Component Access**: Fast lookup and modification of components
4. **Entity Queries**: Find entities with specific component combinations

#### Public Interface:

```cpp
class ECSManager {
public:
    // Entity lifecycle
    uint32_t createEntity();
    bool destroyEntity(uint32_t entityId);
    bool isEntityValid(uint32_t entityId) const;
    
    // Component management (typed)
    bool addComponent(uint32_t entityId, const TransformComponent& component);
    bool addComponent(uint32_t entityId, const PhysicsComponent& component);
    bool addComponent(uint32_t entityId, const BoxColliderComponent& component);
    
    // Component access (templated for type safety)
    template<typename T>
    T* getComponent(uint32_t entityId);
    
    template<typename T>
    bool hasComponent(uint32_t entityId) const;
    
    template<typename T>
    std::vector<uint32_t> getEntitiesWithComponent() const;
};
```

#### Internal Architecture:

```cpp
private:
    // Entity storage
    std::vector<uint32_t> entities;
    uint32_t nextEntityId = 1;
    
    // Component storage (typed for performance)
    std::unordered_map<uint32_t, TransformComponent> transformComponents;
    std::unordered_map<uint32_t, PhysicsComponent> physicsComponents;
    std::unordered_map<uint32_t, BoxColliderComponent> boxColliderComponents;
```

#### Design Benefits:
- **Type Safety**: Templates prevent component type errors at compile time
- **Performance**: Typed storage avoids type erasure overhead
- **Cache Efficiency**: Components of same type stored together
- **Simplicity**: No complex type registration or reflection systems

### CPU Physics Manager

**Purpose**: High-level coordination of CPU physics operations.

**Location**: `src/physics_engine/cpu_physics_engine/managers/physicsmanager/`

#### Responsibilities:
1. **Worker Coordination**: Manages specialized worker components
2. **Resource Management**: Allocates and manages physics resources
3. **Performance Monitoring**: Tracks physics system performance
4. **Configuration Management**: Handles physics system settings

#### Worker Components:
- **RigidBodyWorker**: Handles rigidbody-specific operations
- **PhysicsLayerWorker**: Manages layer interaction logic

## Factories

Factories handle the creation and initialization of complex entities with multiple components.

### RigidBody Component Factory

**Purpose**: Creates and configures individual rigidbody components.

**Location**: `src/physics_engine/cpu_physics_engine/factories/rigidbody_component_factory.h`

#### Responsibilities:
1. **Component Creation**: Instantiate physics components with proper defaults
2. **Property Configuration**: Set up realistic physics properties
3. **Validation**: Ensure component consistency and validity
4. **Template Management**: Provide common component configurations

#### Usage:
```cpp
class RigidBodyComponentFactory {
public:
    // Create components with realistic defaults
    TransformComponent createTransformComponent(float x, float y, float z);
    PhysicsComponent createPhysicsComponent(float mass, bool isStatic = false);
    BoxColliderComponent createBoxColliderComponent(float width, float height, float depth);
    
    // Utility functions
    static float calculateInverseMass(float mass);
    static void validatePhysicsProperties(PhysicsComponent& component);
};
```

### RigidBody Entity Factory

**Purpose**: Creates complete rigidbody entities with all required components.

**Location**: `src/physics_engine/cpu_physics_engine/factories/rigidbody_entity_factory.h`

#### Responsibilities:
1. **Entity Creation**: Create entities with all rigidbody components
2. **ECS Integration**: Interface with ECS Manager for component storage
3. **Batch Operations**: Efficiently create multiple similar entities
4. **Configuration Templates**: Provide preset configurations for common objects

#### Public Interface:
```cpp
class RigidBodyEntityFactory {
public:
    RigidBodyEntityFactory(std::shared_ptr<ECSManager> ecsManager);
    
    // Primary creation method
    uint32_t createRigidBody(float x, float y, float z,
                           float width, float height, float depth,
                           float mass, uint32_t layer);
    
    // Destruction
    bool destroyRigidBody(uint32_t entityId);
    
    // Template creation methods
    uint32_t createStaticBox(float x, float y, float z, float w, float h, float d);
    uint32_t createDynamicBox(float x, float y, float z, float w, float h, float d, float mass);
    uint32_t createKinematicBox(float x, float y, float z, float w, float h, float d);
};
```

#### Creation Process:
1. **Entity Creation**: Request new entity ID from ECS Manager
2. **Component Setup**: Create and configure all required components
3. **Component Registration**: Add components to ECS Manager
4. **Validation**: Verify entity was created correctly
5. **Logging**: Record entity creation for debugging

#### Integration with ECS:
```cpp
uint32_t RigidBodyEntityFactory::createRigidBody(...) {
    // Create entity
    uint32_t entityId = ecsManager->createEntity();
    
    // Create components
    auto transform = componentFactory->createTransformComponent(x, y, z);
    auto physics = componentFactory->createPhysicsComponent(mass);
    auto collider = componentFactory->createBoxColliderComponent(width, height, depth);
    
    // Register with ECS
    ecsManager->addComponent(entityId, transform);
    ecsManager->addComponent(entityId, physics);
    ecsManager->addComponent(entityId, collider);
    
    return entityId;
}
```

## Systems

Systems contain the logic that operates on entities with specific component combinations.

### CPU Physics Collision System

**Purpose**: Handles collision detection, response, and physics integration.

**Location**: `src/physics_engine/cpu_physics_engine/systems/cpu_physics_collision_system.h`

#### Core Responsibilities:
1. **Collision Detection**: Find colliding pairs of rigidbodies
2. **Collision Response**: Resolve collisions with impulse-based methods
3. **Physics Integration**: Update positions and velocities
4. **Layer Filtering**: Respect collision layer interactions
5. **Gravity Application**: Apply gravitational forces

#### Public Interface:
```cpp
class CPUPhysicsCollisionSystem {
public:
    CPUPhysicsCollisionSystem(std::shared_ptr<ECSManager> ecsManager);
    
    // Main simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // Layer management
    void setLayerInteractionCallback(std::function<bool(uint32_t, uint32_t)> callback);
    
    // Configuration
    void setRestitutionDamping(float damping);
    void setPositionCorrection(float percentage);
};
```

#### Physics Pipeline:
1. **Force Application**:
   ```cpp
   void applyGravity(float deltaTime) {
       auto entities = ecsManager->getEntitiesWithComponent<PhysicsComponent>();
       for (uint32_t entityId : entities) {
           auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
           if (physics && physics->useGravity && !physics->isStatic) {
               physics->velocity[1] += gravity.y * deltaTime;
           }
       }
   }
   ```

2. **Collision Detection**:
   ```cpp
   std::vector<CollisionPair> detectCollisions() {
       std::vector<CollisionPair> collisions;
       auto entities = ecsManager->getEntitiesWithComponent<BoxColliderComponent>();
       
       // Broad phase: O(n²) all-pairs test
       for (size_t i = 0; i < entities.size(); ++i) {
           for (size_t j = i + 1; j < entities.size(); ++j) {
               if (testAABBCollision(entities[i], entities[j])) {
                   collisions.push_back({entities[i], entities[j]});
               }
           }
       }
       return collisions;
   }
   ```

3. **Collision Response**:
   - Calculate collision normal and penetration depth
   - Apply impulses to separate objects
   - Handle restitution and friction
   - Update velocities based on collision response

4. **Integration**:
   ```cpp
   void integratePhysics(float deltaTime) {
       auto entities = ecsManager->getEntitiesWithComponent<PhysicsComponent>();
       for (uint32_t entityId : entities) {
           auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
           auto* transform = ecsManager->getComponent<TransformComponent>(entityId);
           
           if (physics && transform && !physics->isStatic) {
               // Update position
               transform->position[0] += physics->velocity[0] * deltaTime;
               transform->position[1] += physics->velocity[1] * deltaTime;
               transform->position[2] += physics->velocity[2] * deltaTime;
           }
       }
   }
   ```

#### Collision Detection Algorithm:

**AABB vs AABB Test**:
```cpp
bool testAABBCollision(uint32_t entityA, uint32_t entityB) {
    auto* transformA = ecsManager->getComponent<TransformComponent>(entityA);
    auto* colliderA = ecsManager->getComponent<BoxColliderComponent>(entityA);
    auto* transformB = ecsManager->getComponent<TransformComponent>(entityB);
    auto* colliderB = ecsManager->getComponent<BoxColliderComponent>(entityB);
    
    // Calculate AABB bounds
    float minXA = transformA->position[0] - colliderA->width * 0.5f;
    float maxXA = transformA->position[0] + colliderA->width * 0.5f;
    // ... similar for Y, Z and entity B
    
    // AABB overlap test
    return (minXA <= maxXB && maxXA >= minXB) &&
           (minYA <= maxYB && maxYA >= minYB) &&
           (minZA <= maxZB && maxZA >= minZB);
}
```

#### Layer Interaction:
```cpp
bool canEntitiesCollide(uint32_t entityA, uint32_t entityB) {
    // Get layer information from legacy rigidbody data
    uint32_t layerA = getLegacyRigidBodyLayer(entityA);
    uint32_t layerB = getLegacyRigidBodyLayer(entityB);
    
    // Check layer interaction callback
    return layerInteractionCallback && layerInteractionCallback(layerA, layerB);
}
```

## Integration Architecture

### Component-System Interaction

The ECS architecture creates clear interfaces between data (components) and behavior (systems):

```cpp
// Systems only operate on specific component combinations
class PhysicsSystem {
    void update(float deltaTime) {
        // Query entities with both Physics and Transform components
        auto entities = ecsManager->getEntitiesWithComponent<PhysicsComponent>();
        for (uint32_t entity : entities) {
            if (ecsManager->hasComponent<TransformComponent>(entity)) {
                processPhysicsEntity(entity, deltaTime);
            }
        }
    }
};
```

### Manager Coordination

Managers coordinate to provide seamless physics operations:

```cpp
// CPUPhysicsEngine coordinates all managers
class CPUPhysicsEngine {
private:
    std::shared_ptr<ECSManager> ecsManager;
    std::shared_ptr<RigidBodyEntityFactory> entityFactory;
    std::shared_ptr<CPUPhysicsCollisionSystem> collisionSystem;
    
public:
    uint32_t createRigidBody(...) {
        // Factory creates entity through ECS Manager
        uint32_t entityId = entityFactory->createRigidBody(...);
        
        // Create legacy wrapper for compatibility
        createLegacyRigidBodyWrapper(entityId);
        
        return entityId;
    }
    
    void updatePhysics(float deltaTime) {
        // System processes entities through ECS Manager
        collisionSystem->updatePhysics(deltaTime);
        
        // Update legacy wrappers
        updateAllLegacyRigidBodyData();
    }
};
```

### Data Flow

1. **Entity Creation**: Factory → ECS Manager → Component Storage
2. **Physics Update**: System → ECS Manager → Component Processing → State Updates
3. **Entity Destruction**: Engine → Factory → ECS Manager → Component Cleanup

### Performance Considerations

#### Memory Layout:
```cpp
// Components stored in separate containers for cache efficiency
std::unordered_map<uint32_t, TransformComponent> transformComponents;
std::unordered_map<uint32_t, PhysicsComponent> physicsComponents;
std::unordered_map<uint32_t, BoxColliderComponent> boxColliderComponents;
```

#### Batch Processing:
```cpp
// Systems process entities in batches
auto entities = ecsManager->getEntitiesWithComponent<PhysicsComponent>();
for (uint32_t entityId : entities) {
    // Process all physics entities together
    processPhysicsEntity(entityId);
}
```

#### Template Optimization:
```cpp
// Templates enable compile-time optimizations
template<typename T>
T* ECSManager::getComponent(uint32_t entityId) {
    // Compiler can optimize specific component types
    if constexpr (std::is_same_v<T, TransformComponent>) {
        auto it = transformComponents.find(entityId);
        return (it != transformComponents.end()) ? &it->second : nullptr;
    }
    // ... other component types
}
```

This component-based architecture provides excellent performance, maintainability, and extensibility for the CPU Physics Engine, while maintaining clean separation of concerns between data storage, entity management, and physics processing.