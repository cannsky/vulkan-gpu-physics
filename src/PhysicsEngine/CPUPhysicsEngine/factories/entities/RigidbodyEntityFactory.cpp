#include "RigidbodyEntityFactory.h"
#include <tuple>

namespace cpu_physics {

RigidBodyEntityFactory::RigidBodyEntityFactory(std::shared_ptr<ECSManager> ecsManager)
    : ecsManager(std::move(ecsManager)) {}

uint32_t RigidBodyEntityFactory::createRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    float mass,
    uint32_t layer) {
    if (!validateRigidBodyParameters(x, y, z, width, height, depth, mass)) return 0;
    uint32_t entityId = ecsManager->createEntity();
    auto transform = componentFactory.createTransformComponent(x, y, z, 1.0f, 1.0f, 1.0f);
    auto physics = componentFactory.createDynamicPhysics(mass);
    auto collider = componentFactory.createBoxCollider(width, height, depth);
    if (!addAllComponents(entityId, transform, physics, collider, layer)) {
        ecsManager->destroyEntity(entityId);
        return 0;
    }
    return entityId;
}

uint32_t RigidBodyEntityFactory::createStaticRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    uint32_t layer) {
    uint32_t entityId = ecsManager->createEntity();
    auto transform = componentFactory.createTransformComponent(x, y, z, 1.0f, 1.0f, 1.0f);
    auto physics = componentFactory.createStaticPhysics();
    auto collider = componentFactory.createBoxCollider(width, height, depth);
    if (!addAllComponents(entityId, transform, physics, collider, layer)) {
        ecsManager->destroyEntity(entityId);
        return 0;
    }
    return entityId;
}

uint32_t RigidBodyEntityFactory::createDynamicRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    float mass,
    uint32_t layer) {
    return createRigidBody(x, y, z, width, height, depth, mass, layer);
}

uint32_t RigidBodyEntityFactory::createKinematicRigidBody(
    float x, float y, float z,
    float width, float height, float depth,
    uint32_t layer) {
    // Treat kinematic as static for now (no forces but can be moved manually)
    return createStaticRigidBody(x, y, z, width, height, depth, layer);
}

uint32_t RigidBodyEntityFactory::createRigidBodyWithComponents(
    const TransformComponent& transform,
    const PhysicsComponent& physics,
    const BoxColliderComponent& collider,
    uint32_t layer) {
    uint32_t entityId = ecsManager->createEntity();
    if (!addAllComponents(entityId, transform, physics, collider, layer)) {
        ecsManager->destroyEntity(entityId);
        return 0;
    }
    return entityId;
}

bool RigidBodyEntityFactory::destroyRigidBody(uint32_t entityId) {
    if (!ecsManager->isValidEntity(entityId)) return false;
    ecsManager->removeComponent<TransformComponent>(entityId);
    ecsManager->removeComponent<PhysicsComponent>(entityId);
    ecsManager->removeComponent<BoxColliderComponent>(entityId);
    return ecsManager->destroyEntity(entityId);
}

bool RigidBodyEntityFactory::isValidRigidBody(uint32_t entityId) const {
    return ecsManager->isValidEntity(entityId) &&
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
    std::vector<uint32_t> ids;
    ids.reserve(specs.size());
    for (const auto& s : specs) {
        ids.push_back(createRigidBody(
            std::get<0>(s), std::get<1>(s), std::get<2>(s),
            std::get<3>(s), std::get<4>(s), std::get<5>(s),
            std::get<6>(s), layer));
    }
    return ids;
}

size_t RigidBodyEntityFactory::getRigidBodyCount() const {
    return ecsManager->getEntityCountWith<TransformComponent, PhysicsComponent, BoxColliderComponent>();
}

std::vector<uint32_t> RigidBodyEntityFactory::getAllRigidBodies() const {
    return ecsManager->getEntitiesWith<TransformComponent, PhysicsComponent, BoxColliderComponent>();
}

bool RigidBodyEntityFactory::addAllComponents(
    uint32_t entityId,
    const TransformComponent& transform,
    const PhysicsComponent& physics,
    const BoxColliderComponent& collider,
    uint32_t layer) {
    ecsManager->addComponent<TransformComponent>(entityId, transform);
    ecsManager->addComponent<PhysicsComponent>(entityId, physics);
    ecsManager->addComponent<BoxColliderComponent>(entityId, collider);
    // TODO: layer component type
    (void)layer;
    return true;
}

bool RigidBodyEntityFactory::validateRigidBodyParameters(
    float x, float y, float z,
    float width, float height, float depth,
    float mass) const {
    (void)x; (void)y; (void)z;
    return width > 0.0f && height > 0.0f && depth > 0.0f && mass >= 0.0f;
}

} // namespace cpu_physics
