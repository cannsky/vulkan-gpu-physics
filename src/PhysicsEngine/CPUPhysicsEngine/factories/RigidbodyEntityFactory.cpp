#include "RigidbodyEntityFactory.h"
#include "../../managers/logmanager/Logger.h"
#include <cmath>

namespace cpu_physics {

RigidBodyEntityFactory::RigidBodyEntityFactory(std::shared_ptr<ECSManager> ecsManager)
    : ecsManager(ecsManager) {
    LOG_INFO(LogCategory::PHYSICS, "Creating RigidBody Entity Factory with ECS integration");
}

uint32_t RigidBodyEntityFactory::createRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    float mass,
    uint32_t layer) {
    
    if (!validateRigidBodyParameters(x, y, z, width, height, depth, mass)) {
        LOG_WARN(LogCategory::RIGIDBODY, "Invalid rigidbody parameters provided");
        return 0;
    }
    
    // Create components
    auto transform = componentFactory.createTransformAt(x, y, z);
    auto physics = (mass > 0.0f) ? 
        componentFactory.createDynamicPhysics(mass) : 
        componentFactory.createStaticPhysics();
    auto collider = componentFactory.createBoxCollider(width, height, depth);
    
    return createRigidBodyWithComponents(transform, physics, collider, layer);
}

uint32_t RigidBodyEntityFactory::createStaticRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    uint32_t layer) {
    
    auto transform = componentFactory.createTransformAt(x, y, z);
    auto physics = componentFactory.createStaticPhysics();
    auto collider = componentFactory.createBoxCollider(width, height, depth);
    
    return createRigidBodyWithComponents(transform, physics, collider, layer);
}

uint32_t RigidBodyEntityFactory::createDynamicRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    float mass,
    uint32_t layer) {
    
    if (mass <= 0.0f) {
        LOG_WARN(LogCategory::RIGIDBODY, "Dynamic rigidbody must have positive mass, using 1.0");
        mass = 1.0f;
    }
    
    auto transform = componentFactory.createTransformAt(x, y, z);
    auto physics = componentFactory.createDynamicPhysics(mass);
    auto collider = componentFactory.createBoxCollider(width, height, depth);
    
    return createRigidBodyWithComponents(transform, physics, collider, layer);
}

uint32_t RigidBodyEntityFactory::createKinematicRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    uint32_t layer) {
    
    auto transform = componentFactory.createTransformAt(x, y, z);
    auto physics = componentFactory.createPhysicsComponent(0.0f, false, false, 0.0f, 0.0f);
    auto collider = componentFactory.createBoxCollider(width, height, depth);
    
    return createRigidBodyWithComponents(transform, physics, collider, layer);
}

uint32_t RigidBodyEntityFactory::createRigidBodyWithComponents(
    const TransformComponent& transform,
    const PhysicsComponent& physics,
    const BoxColliderComponent& collider,
    uint32_t layer) {
    
    // Validate components
    if (!componentFactory.validateTransformComponent(transform) ||
        !componentFactory.validatePhysicsComponent(physics) ||
        !componentFactory.validateBoxColliderComponent(collider)) {
        LOG_WARN(LogCategory::RIGIDBODY, "Invalid component data provided");
        return 0;
    }
    
    // Create entity
    uint32_t entityId = ecsManager->createEntity();
    if (entityId == 0) {
        LOG_ERROR(LogCategory::RIGIDBODY, "Failed to create entity");
        return 0;
    }
    
    // Add components
    if (!addAllComponents(entityId, transform, physics, collider, layer)) {
        ecsManager->destroyEntity(entityId);
        LOG_ERROR(LogCategory::RIGIDBODY, "Failed to add components to entity " + std::to_string(entityId));
        return 0;
    }
    
    LOG_INFO(LogCategory::RIGIDBODY, 
        "Created rigidbody entity " + std::to_string(entityId) + 
        " at (" + std::to_string(transform.position[0]) + 
        ", " + std::to_string(transform.position[1]) + 
        ", " + std::to_string(transform.position[2]) + ")" +
        " with box collider (" + std::to_string(collider.width) + 
        ", " + std::to_string(collider.height) + 
        ", " + std::to_string(collider.depth) + ")");
    
    return entityId;
}

bool RigidBodyEntityFactory::destroyRigidBody(uint32_t entityId) {
    if (!isValidRigidBody(entityId)) {
        return false;
    }
    
    bool result = ecsManager->destroyEntity(entityId);
    if (result) {
        LOG_INFO(LogCategory::RIGIDBODY, "Destroyed rigidbody entity " + std::to_string(entityId));
    }
    
    return result;
}

bool RigidBodyEntityFactory::isValidRigidBody(uint32_t entityId) const {
    return ecsManager->isEntityValid(entityId) &&
           ecsManager->hasComponent<TransformComponent>(entityId) &&
           ecsManager->hasComponent<PhysicsComponent>(entityId) &&
           ecsManager->hasComponent<BoxColliderComponent>(entityId);
}

TransformComponent* RigidBodyEntityFactory::getTransform(uint32_t entityId) {
    return ecsManager->getComponent<TransformComponent>(entityId);
}

PhysicsComponent* RigidBodyEntityFactory::getPhysics(uint32_t entityId) {
    return ecsManager->getComponent<PhysicsComponent>(entityId);
}

BoxColliderComponent* RigidBodyEntityFactory::getCollider(uint32_t entityId) {
    return ecsManager->getComponent<BoxColliderComponent>(entityId);
}

std::vector<uint32_t> RigidBodyEntityFactory::createRigidBodyBatch(
    const std::vector<std::tuple<float, float, float, float, float, float, float>>& specs,
    uint32_t layer) {
    
    std::vector<uint32_t> entities;
    entities.reserve(specs.size());
    
    for (const auto& spec : specs) {
        auto [x, y, z, width, height, depth, mass] = spec;
        uint32_t entity = createRigidBody(x, y, z, width, height, depth, mass, layer);
        if (entity != 0) {
            entities.push_back(entity);
        }
    }
    
    LOG_INFO(LogCategory::RIGIDBODY, 
        "Created batch of " + std::to_string(entities.size()) + 
        " rigidbodies from " + std::to_string(specs.size()) + " specifications");
    
    return entities;
}

size_t RigidBodyEntityFactory::getRigidBodyCount() const {
    auto entities = ecsManager->getEntitiesWithComponent<TransformComponent>();
    size_t count = 0;
    
    for (uint32_t entityId : entities) {
        if (isValidRigidBody(entityId)) {
            count++;
        }
    }
    
    return count;
}

std::vector<uint32_t> RigidBodyEntityFactory::getAllRigidBodies() const {
    auto entities = ecsManager->getEntitiesWithComponent<TransformComponent>();
    std::vector<uint32_t> rigidBodies;
    
    for (uint32_t entityId : entities) {
        if (isValidRigidBody(entityId)) {
            rigidBodies.push_back(entityId);
        }
    }
    
    return rigidBodies;
}

bool RigidBodyEntityFactory::addAllComponents(
    uint32_t entityId,
    const TransformComponent& transform,
    const PhysicsComponent& physics,
    const BoxColliderComponent& collider,
    uint32_t layer) {
    
    if (!ecsManager->addComponent(entityId, transform)) {
        LOG_ERROR(LogCategory::RIGIDBODY, "Failed to add TransformComponent to entity " + std::to_string(entityId));
        return false;
    }
    
    if (!ecsManager->addComponent(entityId, physics)) {
        LOG_ERROR(LogCategory::RIGIDBODY, "Failed to add PhysicsComponent to entity " + std::to_string(entityId));
        return false;
    }
    
    if (!ecsManager->addComponent(entityId, collider)) {
        LOG_ERROR(LogCategory::RIGIDBODY, "Failed to add BoxColliderComponent to entity " + std::to_string(entityId));
        return false;
    }
    
    // TODO: Add layer component when layer system is integrated
    
    return true;
}

bool RigidBodyEntityFactory::validateRigidBodyParameters(
    float x, float y, float z,
    float width, float height, float depth,
    float mass) const {
    
    // Check for finite position values
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
        return false;
    }
    
    // Check for positive dimensions
    if (width <= 0.0f || height <= 0.0f || depth <= 0.0f) {
        return false;
    }
    
    // Check for finite dimensions
    if (!std::isfinite(width) || !std::isfinite(height) || !std::isfinite(depth)) {
        return false;
    }
    
    // Check for non-negative mass
    if (mass < 0.0f || !std::isfinite(mass)) {
        return false;
    }
    
    return true;
}

} // namespace cpu_physics