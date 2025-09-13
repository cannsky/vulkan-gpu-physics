#pragma once

#include "../../../rigidbody/RigidBody.h"
#include "../workers/PhysicsLayerWorker.h"
#include <memory>

/**
 * Factory for creating rigidbodies with proper configuration.
 * Replaces RigidBodySystem with a factory pattern approach.
 */
class RigidBodyFactory {
public:
    static RigidBodyFactory& getInstance();
    
    // Basic shape creation
    std::unique_ptr<RigidBody> createSphere(float x, float y, float z, float radius, 
                                          float mass = 1.0f, PhysicsLayerWorker::LayerId layer = PhysicsLayerWorker::DEFAULT_LAYER);
    
    std::unique_ptr<RigidBody> createBox(float x, float y, float z, float width, float height, float depth, 
                                       float mass = 1.0f, PhysicsLayerWorker::LayerId layer = PhysicsLayerWorker::DEFAULT_LAYER);
    
    std::unique_ptr<RigidBody> createStaticPlane(float y = 0.0f, PhysicsLayerWorker::LayerId layer = PhysicsLayerWorker::DEFAULT_LAYER);
    
    // Advanced creation with custom properties
    std::unique_ptr<RigidBody> createCustomRigidBody(const RigidBody& template_body, PhysicsLayerWorker::LayerId layer = PhysicsLayerWorker::DEFAULT_LAYER);
    
    // Configuration
    void setDefaultMaterial(float restitution, float friction);
    void getDefaultMaterial(float& restitution, float& friction) const;

private:
    RigidBodyFactory() = default;
    ~RigidBodyFactory() = default;
    
    // Non-copyable and non-movable
    RigidBodyFactory(const RigidBodyFactory&) = delete;
    RigidBodyFactory& operator=(const RigidBodyFactory&) = delete;
    RigidBodyFactory(RigidBodyFactory&&) = delete;
    RigidBodyFactory& operator=(RigidBodyFactory&&) = delete;
    
    // Helper methods
    void calculateSphereInertia(RigidBody& body, float radius);
    void calculateBoxInertia(RigidBody& body, float width, float height, float depth);
    void applyDefaultMaterial(RigidBody& body);
    
    // Default material properties
    float defaultRestitution = 0.3f;
    float defaultFriction = 0.7f;
};