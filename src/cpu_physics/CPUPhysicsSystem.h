#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <cstdint>

namespace cpu_physics {

// Box Collider Component (only supported collider type for now)
struct BoxColliderComponent {
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    bool enabled = true;
};

// Transform Component
struct TransformComponent {
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // quaternion (w, x, y, z)
    float scale[3] = {1.0f, 1.0f, 1.0f};
};

// Physics Component
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

// Complete RigidBody entity with all components
struct RigidBodyComponent {
    uint32_t entityId;
    TransformComponent transform;
    PhysicsComponent physics;
    BoxColliderComponent collider;
    uint32_t layer = 0; // Physics layer for collision filtering
    bool hasCollider = true;
};

// Physics Layer for collision filtering
struct PhysicsLayer {
    uint32_t id;
    std::string name;
    std::vector<uint32_t> interactionLayers; // Layers this layer can interact with
};

/**
 * CPU Physics System - Handles complex rigidbody operations with ECS architecture
 * Focuses on rigidbody dynamics, collision detection, and layer-based filtering
 */
class CPUPhysicsSystem {
public:
    CPUPhysicsSystem();
    ~CPUPhysicsSystem() = default;
    
    bool initialize(uint32_t maxRigidBodies = 512);
    void cleanup();
    
    // RigidBody management (ECS-style)
    uint32_t createRigidBody(float x, float y, float z, float width, float height, float depth, float mass = 1.0f, uint32_t layer = 0);
    bool removeRigidBody(uint32_t entityId);
    RigidBodyComponent* getRigidBody(uint32_t entityId);
    
    // Physics simulation
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // Layer system for collision filtering
    uint32_t createLayer(const std::string& name);
    bool setLayerInteraction(uint32_t layer1, uint32_t layer2, bool canInteract);
    bool canLayersInteract(uint32_t layer1, uint32_t layer2) const;
    PhysicsLayer* getLayer(uint32_t layerId);
    size_t getLayerCount() const { return layers.size(); }
    
    // Configuration
    uint32_t getMaxRigidBodies() const { return maxRigidBodies; }
    size_t getRigidBodyCount() const { return rigidBodies.size(); }

private:
    // ECS entity management
    uint32_t nextEntityId = 1;
    uint32_t maxRigidBodies = 512;
    
    // Component storage
    std::unordered_map<uint32_t, RigidBodyComponent> rigidBodies;
    
    // Layer system
    std::unordered_map<uint32_t, PhysicsLayer> layers;
    uint32_t nextLayerId = 1;
    
    // Physics settings
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;
    
    // Physics simulation methods
    void integratePhysics(RigidBodyComponent& body, float deltaTime);
    void detectAndResolveCollisions(float deltaTime);
    bool checkBoxBoxCollision(const RigidBodyComponent& body1, const RigidBodyComponent& body2);
    void resolveCollision(RigidBodyComponent& body1, RigidBodyComponent& body2);
};

} // namespace cpu_physics