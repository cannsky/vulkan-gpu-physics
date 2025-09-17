#pragma once

#include "../../managers/ECSManager/ECSManager.h"
#include "../components/RigidbodyComponentFactory.h"
#include <memory>

namespace cpu_physics {

class RigidBodyEntityFactory {
public:
    RigidBodyEntityFactory(std::shared_ptr<ECSManager> ecsManager);
    ~RigidBodyEntityFactory() = default;
    uint32_t createRigidBody(
        float x, float y, float z,
        float width, float height, float depth,
        float mass = 1.0f,
        uint32_t layer = 0
    );
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
    uint32_t createRigidBodyWithComponents(
        const TransformComponent& transform,
        const PhysicsComponent& physics,
        const BoxColliderComponent& collider,
        uint32_t layer = 0
    );
    bool destroyRigidBody(uint32_t entityId);
    bool isValidRigidBody(uint32_t entityId) const;
    TransformComponent* getTransform(uint32_t entityId);
    PhysicsComponent* getPhysics(uint32_t entityId);
    BoxColliderComponent* getCollider(uint32_t entityId);
    std::vector<uint32_t> createRigidBodyBatch(
        const std::vector<std::tuple<float, float, float, float, float, float, float>>& specs,
        uint32_t layer = 0
    );
    size_t getRigidBodyCount() const;
    std::vector<uint32_t> getAllRigidBodies() const;
private:
    std::shared_ptr<ECSManager> ecsManager;
    RigidBodyComponentFactory componentFactory;
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
