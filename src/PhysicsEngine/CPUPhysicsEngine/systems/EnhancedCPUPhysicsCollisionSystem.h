#pragma once

#include "BaseCPUPhysicsSystem.h"
#include "../interfaces/interfaces.h"
#include "../managers/ECSManager/ECSManager.h"
#include "../components.h"
#include <vector>
#include <memory>
#include <functional>

namespace cpu_physics {

/**
 * Enhanced CPU Physics Collision System implementing the CPUPhysicsSystem interface
 * 
 * This system provides the same functionality as CPUPhysicsCollisionSystem but
 * implements the abstract interface for better modularity and extensibility.
 * It can work with both ECS entities and interface-based entities.
 */
class EnhancedCPUPhysicsCollisionSystem : public BaseCPUPhysicsSystem {
public:
    explicit EnhancedCPUPhysicsCollisionSystem(std::shared_ptr<ECSManager> ecsManager);
    ~EnhancedCPUPhysicsCollisionSystem() override = default;

    // CPUPhysicsSystem interface implementation
    SystemType getType() const override;
    const char* getName() const override;
    Priority getPriority() const override;

    std::vector<interfaces::CPUPhysicsComponent::ComponentType> getRequiredComponents() const override;
    std::vector<interfaces::CPUPhysicsComponent::ComponentType> getOptionalComponents() const override;

    // Legacy compatibility methods (delegates to original collision system)
    void detectCollisions(const std::vector<uint32_t>& entities);
    void resolveCollisions(float deltaTime);

    // Layer system integration
    void setLayerInteractionCallback(
        std::function<bool(uint32_t, uint32_t)> canLayersInteractCallback
    );

    // Configuration
    void setGravity(float x, float y, float z);
    void setBroadPhaseEnabled(bool enabled);
    void setCollisionResponseEnabled(bool enabled);

    // Statistics and debugging
    size_t getLastCollisionCount() const;

    // Collision queries
    std::vector<uint32_t> getCollidingEntities(uint32_t entityId) const;
    bool areEntitiesColliding(uint32_t entityA, uint32_t entityB) const;

protected:
    // BaseCPUPhysicsSystem implementation
    void updateInternal(float deltaTime) override;
    void processEntity(interfaces::CPUPhysicsEntity* entity, float deltaTime) override;

private:
    std::function<bool(uint32_t, uint32_t)> canLayersInteract;
    
    // Collision data
    struct CollisionPair {
        uint32_t entityA;
        uint32_t entityB;
        float penetrationDepth;
        float normal[3]; // Collision normal
        float contactPoint[3]; // Contact point
    };
    
    std::vector<CollisionPair> activeCollisions;
    size_t lastCollisionCount = 0;

    // Physics settings
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;

    // System configuration
    bool broadPhaseEnabled = true;
    bool collisionResponseEnabled = true;

    // Physics integration methods
    void integratePhysics(uint32_t entityId, float deltaTime);
    void applyGravity(uint32_t entityId, float deltaTime);
    void updateTransformFromPhysics(uint32_t entityId, float deltaTime);

    // Collision detection methods
    std::vector<std::pair<uint32_t, uint32_t>> broadPhaseDetection(const std::vector<uint32_t>& entities);
    bool narrowPhaseDetection(uint32_t entityA, uint32_t entityB, CollisionPair& collision);
    bool checkBoxBoxCollision(
        const TransformComponent& transformA, const BoxColliderComponent& colliderA,
        const TransformComponent& transformB, const BoxColliderComponent& colliderB,
        CollisionPair& collision
    );

    // Collision resolution
    void resolveCollision(const CollisionPair& collision);
    void separateEntities(const CollisionPair& collision);
    void applyCollisionImpulse(const CollisionPair& collision);

    // Utility methods
    float calculateDistance(const float* posA, const float* posB) const;
    void calculateCollisionNormal(
        const TransformComponent& transformA, const BoxColliderComponent& colliderA,
        const TransformComponent& transformB, const BoxColliderComponent& colliderB,
        float* normal
    ) const;
    bool canEntitiesCollide(uint32_t entityA, uint32_t entityB) const;

    // AABB (Axis-Aligned Bounding Box) helpers
    struct AABB {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    AABB calculateAABB(const TransformComponent& transform, const BoxColliderComponent& collider) const;
    bool aabbOverlap(const AABB& a, const AABB& b) const;
};

} // namespace cpu_physics