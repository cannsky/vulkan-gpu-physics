#pragma once

#include "../managers/ECSManager/ECSManager.h"
#include "RigidbodyComponentFactory.h"
#include <memory>

namespace cpu_physics {

/**
 * RigidBody Entity Factory - Creates complete rigidbody entities with all necessary components
 * 
 * This factory handles the creation of full rigidbody entities by:
 * 1. Creating an entity in the ECS manager
 * 2. Adding all required components (Transform, Physics, Collider)
 * 3. Configuring the components based on parameters
 * 4. Setting up physics layer assignments
 */
class RigidBodyEntityFactory {
public:
    RigidBodyEntityFactory(std::shared_ptr<ECSManager> ecsManager);
    ~RigidBodyEntityFactory() = default;
    
    // Complete rigidbody creation
    uint32_t createRigidBody(
        float x, float y, float z,
        float width, float height, float depth,
        float mass = 1.0f,
        uint32_t layer = 0
    );
    
    // Preset rigidbody types
    uint32_t createStaticRigidBody(
        float x, float y, float z,
        float width, float height, float depth,
        uint32_t layer = 0
    );
    
    uint32_t createDynamicRigidBody(
        float x, float y, float z,
        float width, float height, float depth,
        float mass = 1.0f,
        uint32_t layer = 0
    );
    
    uint32_t createKinematicRigidBody(
        float x, float y, float z,
        float width, float height, float depth,
        uint32_t layer = 0
    );
    
    // Component-specific creation
    uint32_t createRigidBodyWithComponents(
        const TransformComponent& transform,
        const PhysicsComponent& physics,
        const BoxColliderComponent& collider,
        uint32_t layer = 0
    );
    
    // Entity management
    bool destroyRigidBody(uint32_t entityId);
    bool isValidRigidBody(uint32_t entityId) const;
    
    // Component access helpers
    TransformComponent* getTransform(uint32_t entityId);
    PhysicsComponent* getPhysics(uint32_t entityId);
    BoxColliderComponent* getCollider(uint32_t entityId);
    
    // Batch operations
    std::vector<uint32_t> createRigidBodyBatch(
        const std::vector<std::tuple<float, float, float, float, float, float, float>>& specs,
        uint32_t layer = 0
    );
    
    // Statistics
    size_t getRigidBodyCount() const;
    std::vector<uint32_t> getAllRigidBodies() const;

private:
    std::shared_ptr<ECSManager> ecsManager;
    RigidBodyComponentFactory componentFactory;
    
    // Helper methods
    bool addAllComponents(
        uint32_t entityId,
        const TransformComponent& transform,
        const PhysicsComponent& physics,
        const BoxColliderComponent& collider,
        uint32_t layer
    );
    
    bool validateRigidBodyParameters(
        float x, float y, float z,
        float width, float height, float depth,
        float mass
    ) const;
};

} // namespace cpu_physics