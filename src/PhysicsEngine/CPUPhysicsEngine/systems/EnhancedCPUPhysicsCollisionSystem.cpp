#include "EnhancedCPUPhysicsCollisionSystem.h"
#include "../../managers/logmanager/Logger.h"
#include <cmath>
#include <algorithm>
#include <chrono>

namespace cpu_physics {

EnhancedCPUPhysicsCollisionSystem::EnhancedCPUPhysicsCollisionSystem(std::shared_ptr<ECSManager> ecsManager)
    : BaseCPUPhysicsSystem(ecsManager) {
    LOG_INFO(LogCategory::PHYSICS, "Creating Enhanced CPU Physics Collision System with interface support");
}

interfaces::CPUPhysicsSystem::SystemType EnhancedCPUPhysicsCollisionSystem::getType() const {
    return SystemType::COLLISION;
}

const char* EnhancedCPUPhysicsCollisionSystem::getName() const {
    return "EnhancedCPUPhysicsCollisionSystem";
}

interfaces::CPUPhysicsSystem::Priority EnhancedCPUPhysicsCollisionSystem::getPriority() const {
    return Priority::HIGH; // Collision systems typically run with high priority
}

std::vector<interfaces::CPUPhysicsComponent::ComponentType> EnhancedCPUPhysicsCollisionSystem::getRequiredComponents() const {
    return {
        interfaces::CPUPhysicsComponent::ComponentType::TRANSFORM,
        interfaces::CPUPhysicsComponent::ComponentType::PHYSICS,
        interfaces::CPUPhysicsComponent::ComponentType::BOX_COLLIDER
    };
}

std::vector<interfaces::CPUPhysicsComponent::ComponentType> EnhancedCPUPhysicsCollisionSystem::getOptionalComponents() const {
    return {}; // No optional components for now
}

void EnhancedCPUPhysicsCollisionSystem::updateInternal(float deltaTime) {
    // Get all entities with the required components for physics
    auto entities = getEntitiesWithRequiredComponents();
    
    // Clear previous collision data
    activeCollisions.clear();
    
    // Physics integration (apply forces, update velocities)
    for (uint32_t entityId : entities) {
        integratePhysics(entityId, deltaTime);
    }
    
    // Collision detection
    if (!entities.empty()) {
        detectCollisions(entities);
    }
    
    // Collision resolution
    if (!activeCollisions.empty()) {
        resolveCollisions(deltaTime);
    }
    
    // Update transforms based on physics
    for (uint32_t entityId : entities) {
        updateTransformFromPhysics(entityId, deltaTime);
    }
    
    // Update statistics
    lastCollisionCount = activeCollisions.size();
    
    if (entities.size() > 0) {
        LOG_DEBUG(LogCategory::PHYSICS, 
            "Enhanced collision system update: " + std::to_string(entities.size()) + 
            " entities, " + std::to_string(lastCollisionCount) + " collisions");
    }
}

void EnhancedCPUPhysicsCollisionSystem::processEntity(interfaces::CPUPhysicsEntity* entity, float deltaTime) {
    // This method is called when processing interface-based entities
    // For now, we'll log that this functionality is not yet implemented
    LOG_DEBUG(LogCategory::PHYSICS, 
        "Enhanced collision system: Interface-based entity processing not yet implemented for entity " + 
        std::to_string(entity->getId()));
}

void EnhancedCPUPhysicsCollisionSystem::detectCollisions(const std::vector<uint32_t>& entities) {
    if (!broadPhaseEnabled || entities.size() < 2) {
        return;
    }

    // Broad phase collision detection
    auto potentialPairs = broadPhaseDetection(entities);
    
    // Narrow phase collision detection
    for (const auto& pair : potentialPairs) {
        uint32_t entityA = pair.first;
        uint32_t entityB = pair.second;
        
        // Check if entities can collide (layer filtering)
        if (!canEntitiesCollide(entityA, entityB)) {
            continue;
        }
        
        CollisionPair collision;
        if (narrowPhaseDetection(entityA, entityB, collision)) {
            activeCollisions.push_back(collision);
        }
    }
}

void EnhancedCPUPhysicsCollisionSystem::resolveCollisions(float deltaTime) {
    if (!collisionResponseEnabled) {
        return;
    }

    for (const auto& collision : activeCollisions) {
        resolveCollision(collision);
    }
}

void EnhancedCPUPhysicsCollisionSystem::setLayerInteractionCallback(
    std::function<bool(uint32_t, uint32_t)> canLayersInteractCallback) {
    canLayersInteract = canLayersInteractCallback;
}

void EnhancedCPUPhysicsCollisionSystem::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    LOG_INFO(LogCategory::PHYSICS, 
        "Enhanced collision system gravity set to (" + 
        std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

void EnhancedCPUPhysicsCollisionSystem::setBroadPhaseEnabled(bool enabled) {
    broadPhaseEnabled = enabled;
}

void EnhancedCPUPhysicsCollisionSystem::setCollisionResponseEnabled(bool enabled) {
    collisionResponseEnabled = enabled;
}

size_t EnhancedCPUPhysicsCollisionSystem::getLastCollisionCount() const {
    return lastCollisionCount;
}

std::vector<uint32_t> EnhancedCPUPhysicsCollisionSystem::getCollidingEntities(uint32_t entityId) const {
    std::vector<uint32_t> collidingEntities;
    
    for (const auto& collision : activeCollisions) {
        if (collision.entityA == entityId) {
            collidingEntities.push_back(collision.entityB);
        } else if (collision.entityB == entityId) {
            collidingEntities.push_back(collision.entityA);
        }
    }
    
    return collidingEntities;
}

bool EnhancedCPUPhysicsCollisionSystem::areEntitiesColliding(uint32_t entityA, uint32_t entityB) const {
    for (const auto& collision : activeCollisions) {
        if ((collision.entityA == entityA && collision.entityB == entityB) ||
            (collision.entityA == entityB && collision.entityB == entityA)) {
            return true;
        }
    }
    return false;
}

// Private implementation methods (simplified versions of collision detection/resolution)
void EnhancedCPUPhysicsCollisionSystem::integratePhysics(uint32_t entityId, float deltaTime) {
    auto ecsManager = getECSManager();
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    auto* transform = ecsManager->getComponent<TransformComponent>(entityId);
    
    if (!physics || !transform || physics->isStatic) {
        return;
    }
    
    // Apply gravity
    if (physics->useGravity) {
        physics->velocity[0] += gravity.x * deltaTime;
        physics->velocity[1] += gravity.y * deltaTime;
        physics->velocity[2] += gravity.z * deltaTime;
    }
}

void EnhancedCPUPhysicsCollisionSystem::applyGravity(uint32_t entityId, float deltaTime) {
    auto ecsManager = getECSManager();
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    
    if (physics && physics->useGravity && !physics->isStatic) {
        physics->velocity[1] += gravity.y * deltaTime;
    }
}

void EnhancedCPUPhysicsCollisionSystem::updateTransformFromPhysics(uint32_t entityId, float deltaTime) {
    auto ecsManager = getECSManager();
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    auto* transform = ecsManager->getComponent<TransformComponent>(entityId);
    
    if (!physics || !transform || physics->isStatic) {
        return;
    }
    
    // Update position based on velocity
    transform->position[0] += physics->velocity[0] * deltaTime;
    transform->position[1] += physics->velocity[1] * deltaTime;
    transform->position[2] += physics->velocity[2] * deltaTime;
}

std::vector<std::pair<uint32_t, uint32_t>> EnhancedCPUPhysicsCollisionSystem::broadPhaseDetection(const std::vector<uint32_t>& entities) {
    std::vector<std::pair<uint32_t, uint32_t>> pairs;
    
    // Simple O(n²) broad phase - could be optimized with spatial partitioning
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            pairs.emplace_back(entities[i], entities[j]);
        }
    }
    
    return pairs;
}

bool EnhancedCPUPhysicsCollisionSystem::narrowPhaseDetection(uint32_t entityA, uint32_t entityB, CollisionPair& collision) {
    auto ecsManager = getECSManager();
    
    auto* transformA = ecsManager->getComponent<TransformComponent>(entityA);
    auto* transformB = ecsManager->getComponent<TransformComponent>(entityB);
    auto* colliderA = ecsManager->getComponent<BoxColliderComponent>(entityA);
    auto* colliderB = ecsManager->getComponent<BoxColliderComponent>(entityB);
    
    if (!transformA || !transformB || !colliderA || !colliderB) {
        return false;
    }
    
    return checkBoxBoxCollision(*transformA, *colliderA, *transformB, *colliderB, collision);
}

bool EnhancedCPUPhysicsCollisionSystem::checkBoxBoxCollision(
    const TransformComponent& transformA, const BoxColliderComponent& colliderA,
    const TransformComponent& transformB, const BoxColliderComponent& colliderB,
    CollisionPair& collision) {
    
    AABB aabbA = calculateAABB(transformA, colliderA);
    AABB aabbB = calculateAABB(transformB, colliderB);
    
    if (!aabbOverlap(aabbA, aabbB)) {
        return false;
    }
    
    // Fill collision information
    collision.entityA = 0; // Will be set by caller
    collision.entityB = 0; // Will be set by caller
    collision.penetrationDepth = 0.1f; // Simplified
    collision.normal[0] = 1.0f; // Simplified normal
    collision.normal[1] = 0.0f;
    collision.normal[2] = 0.0f;
    collision.contactPoint[0] = (transformA.position[0] + transformB.position[0]) * 0.5f;
    collision.contactPoint[1] = (transformA.position[1] + transformB.position[1]) * 0.5f;
    collision.contactPoint[2] = (transformA.position[2] + transformB.position[2]) * 0.5f;
    
    return true;
}

void EnhancedCPUPhysicsCollisionSystem::resolveCollision(const CollisionPair& collision) {
    // Simplified collision resolution
    separateEntities(collision);
    applyCollisionImpulse(collision);
}

void EnhancedCPUPhysicsCollisionSystem::separateEntities(const CollisionPair& collision) {
    auto ecsManager = getECSManager();
    auto* transformA = ecsManager->getComponent<TransformComponent>(collision.entityA);
    auto* transformB = ecsManager->getComponent<TransformComponent>(collision.entityB);
    
    if (!transformA || !transformB) {
        return;
    }
    
    // Simple separation along collision normal
    float separationDistance = collision.penetrationDepth * 0.5f;
    transformA->position[0] -= collision.normal[0] * separationDistance;
    transformA->position[1] -= collision.normal[1] * separationDistance;
    transformA->position[2] -= collision.normal[2] * separationDistance;
    
    transformB->position[0] += collision.normal[0] * separationDistance;
    transformB->position[1] += collision.normal[1] * separationDistance;
    transformB->position[2] += collision.normal[2] * separationDistance;
}

void EnhancedCPUPhysicsCollisionSystem::applyCollisionImpulse(const CollisionPair& collision) {
    auto ecsManager = getECSManager();
    auto* physicsA = ecsManager->getComponent<PhysicsComponent>(collision.entityA);
    auto* physicsB = ecsManager->getComponent<PhysicsComponent>(collision.entityB);
    
    if (!physicsA || !physicsB) {
        return;
    }
    
    // Simple impulse application (simplified)
    float impulse = 0.5f;
    physicsA->velocity[0] -= collision.normal[0] * impulse;
    physicsA->velocity[1] -= collision.normal[1] * impulse;
    physicsA->velocity[2] -= collision.normal[2] * impulse;
    
    physicsB->velocity[0] += collision.normal[0] * impulse;
    physicsB->velocity[1] += collision.normal[1] * impulse;
    physicsB->velocity[2] += collision.normal[2] * impulse;
}

float EnhancedCPUPhysicsCollisionSystem::calculateDistance(const float* posA, const float* posB) const {
    float dx = posA[0] - posB[0];
    float dy = posA[1] - posB[1];
    float dz = posA[2] - posB[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

void EnhancedCPUPhysicsCollisionSystem::calculateCollisionNormal(
    const TransformComponent& transformA, const BoxColliderComponent& colliderA,
    const TransformComponent& transformB, const BoxColliderComponent& colliderB,
    float* normal) const {
    
    // Simplified normal calculation
    normal[0] = transformB.position[0] - transformA.position[0];
    normal[1] = transformB.position[1] - transformA.position[1];
    normal[2] = transformB.position[2] - transformA.position[2];
    
    // Normalize
    float length = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    if (length > 0.0f) {
        normal[0] /= length;
        normal[1] /= length;
        normal[2] /= length;
    }
}

bool EnhancedCPUPhysicsCollisionSystem::canEntitiesCollide(uint32_t entityA, uint32_t entityB) const {
    if (canLayersInteract) {
        // Get layer information from entities and check if they can interact
        // For now, assume they can interact
        return canLayersInteract(0, 0); // Simplified
    }
    return true;
}

EnhancedCPUPhysicsCollisionSystem::AABB EnhancedCPUPhysicsCollisionSystem::calculateAABB(
    const TransformComponent& transform, const BoxColliderComponent& collider) const {
    
    AABB aabb;
    float halfWidth = collider.width * transform.scale[0] * 0.5f;
    float halfHeight = collider.height * transform.scale[1] * 0.5f;
    float halfDepth = collider.depth * transform.scale[2] * 0.5f;
    
    aabb.minX = transform.position[0] - halfWidth;
    aabb.maxX = transform.position[0] + halfWidth;
    aabb.minY = transform.position[1] - halfHeight;
    aabb.maxY = transform.position[1] + halfHeight;
    aabb.minZ = transform.position[2] - halfDepth;
    aabb.maxZ = transform.position[2] + halfDepth;
    
    return aabb;
}

bool EnhancedCPUPhysicsCollisionSystem::aabbOverlap(const AABB& a, const AABB& b) const {
    return (a.minX <= b.maxX && a.maxX >= b.minX) &&
           (a.minY <= b.maxY && a.maxY >= b.minY) &&
           (a.minZ <= b.maxZ && a.maxZ >= b.minZ);
}

} // namespace cpu_physics