#pragma once

#include "../managers/ECSManager/ECSManager.h"
#include "../components.h" // For component definitions
#include <vector>
#include <memory>
#include <functional>

namespace cpu_physics {

/**
 * CPU Physics Collision System - Handles collision detection and resolution for ECS entities
 * 
 * This system operates on entities that have Transform, Physics, and Collider components.
 * It implements:
 * - Broad phase collision detection (spatial partitioning)
 * - Narrow phase collision detection (shape-specific tests)
 * - Collision response and resolution
 * - Layer-based filtering
 */
class CPUPhysicsCollisionSystem {
public:
    CPUPhysicsCollisionSystem(std::shared_ptr<ECSManager> ecsManager);
    ~CPUPhysicsCollisionSystem() = default;
    
    // System update
    void update(float deltaTime);
    
    // Collision detection
    void detectCollisions(const std::vector<uint32_t>& entities);
    void resolveCollisions(float deltaTime);
    
    // Layer system integration
    void setLayerInteractionCallback(
        std::function<bool(uint32_t, uint32_t)> canLayersInteractCallback
    );
    
    // Configuration
    void setGravity(float x, float y, float z);
    void setBroadPhaseEnabled(bool enabled) { broadPhaseEnabled = enabled; }
    void setCollisionResponseEnabled(bool enabled) { collisionResponseEnabled = enabled; }
    
    // Statistics and debugging
    size_t getLastCollisionCount() const { return lastCollisionCount; }
    float getLastUpdateTime() const { return lastUpdateTime; }
    
    // Collision queries
    std::vector<uint32_t> getCollidingEntities(uint32_t entityId) const;
    bool areEntitiesColliding(uint32_t entityA, uint32_t entityB) const;

private:
    std::shared_ptr<ECSManager> ecsManager;
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
    float lastUpdateTime = 0.0f;
    
    // Physics settings
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;
    
    // System configuration
    bool broadPhaseEnabled = true;
    bool collisionResponseEnabled = true;
    
    // Physics integration
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