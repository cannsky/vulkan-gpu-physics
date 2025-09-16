#include "cpu_physics_collision_system.h"
#include "../../managers/logmanager/Logger.h"
#include <cmath>
#include <algorithm>
#include <chrono>
#include <functional>

namespace cpu_physics {

CPUPhysicsCollisionSystem::CPUPhysicsCollisionSystem(std::shared_ptr<ECSManager> ecsManager)
    : ecsManager(ecsManager) {
    LOG_INFO(LogCategory::PHYSICS, "Creating CPU Physics Collision System with ECS integration");
}

void CPUPhysicsCollisionSystem::update(float deltaTime) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Get all entities with the required components for physics
    auto entities = ecsManager->getEntitiesWithComponent<TransformComponent>();
    
    // Filter entities that have all required components (Transform, Physics, Collider)
    std::vector<uint32_t> physicsEntities;
    for (uint32_t entityId : entities) {
        if (ecsManager->hasComponent<TransformComponent>(entityId) &&
            ecsManager->hasComponent<PhysicsComponent>(entityId) &&
            ecsManager->hasComponent<BoxColliderComponent>(entityId)) {
            physicsEntities.push_back(entityId);
        }
    }
    
    // Clear previous collision data
    activeCollisions.clear();
    
    // Physics integration (apply forces, update velocities)
    for (uint32_t entityId : physicsEntities) {
        integratePhysics(entityId, deltaTime);
    }
    
    // Collision detection
    if (!physicsEntities.empty()) {
        detectCollisions(physicsEntities);
    }
    
    // Collision resolution
    if (!activeCollisions.empty()) {
        resolveCollisions(deltaTime);
    }
    
    // Update transforms based on physics
    for (uint32_t entityId : physicsEntities) {
        updateTransformFromPhysics(entityId, deltaTime);
    }
    
    // Update statistics
    lastCollisionCount = activeCollisions.size();
    auto endTime = std::chrono::high_resolution_clock::now();
    lastUpdateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    if (physicsEntities.size() > 0) {
        LOG_DEBUG(LogCategory::PHYSICS, 
            "Collision system update: " + std::to_string(physicsEntities.size()) + 
            " entities, " + std::to_string(lastCollisionCount) + 
            " collisions, " + std::to_string(lastUpdateTime) + "ms");
    }
}

void CPUPhysicsCollisionSystem::detectCollisions(const std::vector<uint32_t>& entities) {
    std::vector<std::pair<uint32_t, uint32_t>> candidatePairs;
    
    if (broadPhaseEnabled) {
        candidatePairs = broadPhaseDetection(entities);
    } else {
        // Brute force - test all pairs
        for (size_t i = 0; i < entities.size(); i++) {
            for (size_t j = i + 1; j < entities.size(); j++) {
                candidatePairs.emplace_back(entities[i], entities[j]);
            }
        }
    }
    
    // Narrow phase detection
    for (const auto& pair : candidatePairs) {
        if (canEntitiesCollide(pair.first, pair.second)) {
            CollisionPair collision;
            if (narrowPhaseDetection(pair.first, pair.second, collision)) {
                activeCollisions.push_back(collision);
            }
        }
    }
}

void CPUPhysicsCollisionSystem::resolveCollisions(float deltaTime) {
    for (const auto& collision : activeCollisions) {
        if (collisionResponseEnabled) {
            resolveCollision(collision);
        }
    }
}

void CPUPhysicsCollisionSystem::setLayerInteractionCallback(
    std::function<bool(uint32_t, uint32_t)> canLayersInteractCallback) {
    canLayersInteract = canLayersInteractCallback;
}

void CPUPhysicsCollisionSystem::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    LOG_INFO(LogCategory::PHYSICS, 
        "Collision system gravity set to (" + 
        std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

std::vector<uint32_t> CPUPhysicsCollisionSystem::getCollidingEntities(uint32_t entityId) const {
    std::vector<uint32_t> colliding;
    
    for (const auto& collision : activeCollisions) {
        if (collision.entityA == entityId) {
            colliding.push_back(collision.entityB);
        } else if (collision.entityB == entityId) {
            colliding.push_back(collision.entityA);
        }
    }
    
    return colliding;
}

bool CPUPhysicsCollisionSystem::areEntitiesColliding(uint32_t entityA, uint32_t entityB) const {
    for (const auto& collision : activeCollisions) {
        if ((collision.entityA == entityA && collision.entityB == entityB) ||
            (collision.entityA == entityB && collision.entityB == entityA)) {
            return true;
        }
    }
    return false;
}

void CPUPhysicsCollisionSystem::integratePhysics(uint32_t entityId, float deltaTime) {
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    if (!physics || physics->isStatic) {
        return;
    }
    
    // Apply gravity
    if (physics->useGravity) {
        applyGravity(entityId, deltaTime);
    }
    
    // Apply damping (air resistance)
    const float damping = 0.99f;
    for (int i = 0; i < 3; i++) {
        physics->velocity[i] *= damping;
        physics->angularVelocity[i] *= damping;
    }
}

void CPUPhysicsCollisionSystem::applyGravity(uint32_t entityId, float deltaTime) {
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    if (!physics || physics->isStatic || physics->invMass <= 0.0f) {
        return;
    }
    
    // F = ma, a = F/m, where F is gravity force
    physics->velocity[0] += gravity.x * deltaTime;
    physics->velocity[1] += gravity.y * deltaTime;
    physics->velocity[2] += gravity.z * deltaTime;
}

void CPUPhysicsCollisionSystem::updateTransformFromPhysics(uint32_t entityId, float deltaTime) {
    auto* transform = ecsManager->getComponent<TransformComponent>(entityId);
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    
    if (!transform || !physics || physics->isStatic) {
        return;
    }
    
    // Update position based on velocity
    transform->position[0] += physics->velocity[0] * deltaTime;
    transform->position[1] += physics->velocity[1] * deltaTime;
    transform->position[2] += physics->velocity[2] * deltaTime;
    
    // TODO: Update rotation based on angular velocity (quaternion integration)
}

std::vector<std::pair<uint32_t, uint32_t>> CPUPhysicsCollisionSystem::broadPhaseDetection(const std::vector<uint32_t>& entities) {
    std::vector<std::pair<uint32_t, uint32_t>> candidatePairs;
    
    // Simple AABB overlap test for broad phase
    for (size_t i = 0; i < entities.size(); i++) {
        for (size_t j = i + 1; j < entities.size(); j++) {
            uint32_t entityA = entities[i];
            uint32_t entityB = entities[j];
            
            auto* transformA = ecsManager->getComponent<TransformComponent>(entityA);
            auto* transformB = ecsManager->getComponent<TransformComponent>(entityB);
            auto* colliderA = ecsManager->getComponent<BoxColliderComponent>(entityA);
            auto* colliderB = ecsManager->getComponent<BoxColliderComponent>(entityB);
            
            if (transformA && transformB && colliderA && colliderB) {
                AABB aabbA = calculateAABB(*transformA, *colliderA);
                AABB aabbB = calculateAABB(*transformB, *colliderB);
                
                if (aabbOverlap(aabbA, aabbB)) {
                    candidatePairs.emplace_back(entityA, entityB);
                }
            }
        }
    }
    
    return candidatePairs;
}

bool CPUPhysicsCollisionSystem::narrowPhaseDetection(uint32_t entityA, uint32_t entityB, CollisionPair& collision) {
    auto* transformA = ecsManager->getComponent<TransformComponent>(entityA);
    auto* transformB = ecsManager->getComponent<TransformComponent>(entityB);
    auto* colliderA = ecsManager->getComponent<BoxColliderComponent>(entityA);
    auto* colliderB = ecsManager->getComponent<BoxColliderComponent>(entityB);
    
    if (!transformA || !transformB || !colliderA || !colliderB) {
        return false;
    }
    
    if (!colliderA->enabled || !colliderB->enabled) {
        return false;
    }
    
    collision.entityA = entityA;
    collision.entityB = entityB;
    
    return checkBoxBoxCollision(*transformA, *colliderA, *transformB, *colliderB, collision);
}

bool CPUPhysicsCollisionSystem::checkBoxBoxCollision(
    const TransformComponent& transformA, const BoxColliderComponent& colliderA,
    const TransformComponent& transformB, const BoxColliderComponent& colliderB,
    CollisionPair& collision) {
    
    // Calculate half extents
    float halfExtentsA[3] = {
        colliderA.width * transformA.scale[0] * 0.5f,
        colliderA.height * transformA.scale[1] * 0.5f,
        colliderA.depth * transformA.scale[2] * 0.5f
    };
    
    float halfExtentsB[3] = {
        colliderB.width * transformB.scale[0] * 0.5f,
        colliderB.height * transformB.scale[1] * 0.5f,
        colliderB.depth * transformB.scale[2] * 0.5f
    };
    
    // Calculate distance between centers
    float distance[3];
    for (int i = 0; i < 3; i++) {
        distance[i] = std::abs(transformA.position[i] - transformB.position[i]);
    }
    
    // Check for separation along each axis
    float penetration[3];
    bool colliding = true;
    
    for (int i = 0; i < 3; i++) {
        float totalExtent = halfExtentsA[i] + halfExtentsB[i];
        if (distance[i] >= totalExtent) {
            colliding = false;
            break;
        }
        penetration[i] = totalExtent - distance[i];
    }
    
    if (!colliding) {
        return false;
    }
    
    // Find the axis with minimum penetration (separation axis)
    int minAxis = 0;
    for (int i = 1; i < 3; i++) {
        if (penetration[i] < penetration[minAxis]) {
            minAxis = i;
        }
    }
    
    collision.penetrationDepth = penetration[minAxis];
    
    // Calculate collision normal
    for (int i = 0; i < 3; i++) {
        collision.normal[i] = 0.0f;
    }
    collision.normal[minAxis] = (transformA.position[minAxis] > transformB.position[minAxis]) ? 1.0f : -1.0f;
    
    // Calculate contact point (midpoint of overlapping region)
    for (int i = 0; i < 3; i++) {
        collision.contactPoint[i] = (transformA.position[i] + transformB.position[i]) * 0.5f;
    }
    
    return true;
}

void CPUPhysicsCollisionSystem::resolveCollision(const CollisionPair& collision) {
    separateEntities(collision);
    applyCollisionImpulse(collision);
}

void CPUPhysicsCollisionSystem::separateEntities(const CollisionPair& collision) {
    auto* transformA = ecsManager->getComponent<TransformComponent>(collision.entityA);
    auto* transformB = ecsManager->getComponent<TransformComponent>(collision.entityB);
    auto* physicsA = ecsManager->getComponent<PhysicsComponent>(collision.entityA);
    auto* physicsB = ecsManager->getComponent<PhysicsComponent>(collision.entityB);
    
    if (!transformA || !transformB || !physicsA || !physicsB) {
        return;
    }
    
    // Calculate separation based on inverse masses
    float totalInvMass = physicsA->invMass + physicsB->invMass;
    if (totalInvMass <= 0.0f) {
        return; // Both static
    }
    
    float separationA = (physicsA->invMass / totalInvMass) * collision.penetrationDepth * 0.5f;
    float separationB = (physicsB->invMass / totalInvMass) * collision.penetrationDepth * 0.5f;
    
    // Separate entities along collision normal
    if (!physicsA->isStatic) {
        for (int i = 0; i < 3; i++) {
            transformA->position[i] += collision.normal[i] * separationA;
        }
    }
    
    if (!physicsB->isStatic) {
        for (int i = 0; i < 3; i++) {
            transformB->position[i] -= collision.normal[i] * separationB;
        }
    }
}

void CPUPhysicsCollisionSystem::applyCollisionImpulse(const CollisionPair& collision) {
    auto* physicsA = ecsManager->getComponent<PhysicsComponent>(collision.entityA);
    auto* physicsB = ecsManager->getComponent<PhysicsComponent>(collision.entityB);
    
    if (!physicsA || !physicsB) {
        return;
    }
    
    // Calculate relative velocity
    float relativeVelocity[3];
    for (int i = 0; i < 3; i++) {
        relativeVelocity[i] = physicsA->velocity[i] - physicsB->velocity[i];
    }
    
    // Calculate relative velocity along normal
    float velAlongNormal = 0.0f;
    for (int i = 0; i < 3; i++) {
        velAlongNormal += relativeVelocity[i] * collision.normal[i];
    }
    
    // Objects separating, no impulse needed
    if (velAlongNormal > 0.0f) {
        return;
    }
    
    // Calculate restitution (bounce)
    float restitution = std::min(physicsA->restitution, physicsB->restitution);
    
    // Calculate impulse magnitude
    float impulseMagnitude = -(1.0f + restitution) * velAlongNormal;
    impulseMagnitude /= (physicsA->invMass + physicsB->invMass);
    
    // Apply impulse
    if (!physicsA->isStatic) {
        for (int i = 0; i < 3; i++) {
            physicsA->velocity[i] += impulseMagnitude * physicsA->invMass * collision.normal[i];
        }
    }
    
    if (!physicsB->isStatic) {
        for (int i = 0; i < 3; i++) {
            physicsB->velocity[i] -= impulseMagnitude * physicsB->invMass * collision.normal[i];
        }
    }
}

float CPUPhysicsCollisionSystem::calculateDistance(const float* posA, const float* posB) const {
    float dx = posA[0] - posB[0];
    float dy = posA[1] - posB[1];
    float dz = posA[2] - posB[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool CPUPhysicsCollisionSystem::canEntitiesCollide(uint32_t entityA, uint32_t entityB) const {
    // TODO: Implement layer-based filtering when layer components are added
    if (canLayersInteract) {
        // For now, assume layer 0 for all entities
        return canLayersInteract(0, 0);
    }
    return true;
}

CPUPhysicsCollisionSystem::AABB CPUPhysicsCollisionSystem::calculateAABB(
    const TransformComponent& transform, const BoxColliderComponent& collider) const {
    
    float halfWidth = collider.width * transform.scale[0] * 0.5f;
    float halfHeight = collider.height * transform.scale[1] * 0.5f;
    float halfDepth = collider.depth * transform.scale[2] * 0.5f;
    
    AABB aabb;
    aabb.minX = transform.position[0] - halfWidth;
    aabb.maxX = transform.position[0] + halfWidth;
    aabb.minY = transform.position[1] - halfHeight;
    aabb.maxY = transform.position[1] + halfHeight;
    aabb.minZ = transform.position[2] - halfDepth;
    aabb.maxZ = transform.position[2] + halfDepth;
    
    return aabb;
}

bool CPUPhysicsCollisionSystem::aabbOverlap(const AABB& a, const AABB& b) const {
    return (a.minX <= b.maxX && a.maxX >= b.minX) &&
           (a.minY <= b.maxY && a.maxY >= b.minY) &&
           (a.minZ <= b.maxZ && a.maxZ >= b.minZ);
}

} // namespace cpu_physics