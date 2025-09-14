#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <cstdint>
#include <functional>

// ECS components and systems
#include "components.h"
#include "managers/ecs_manager.h"
#include "factories/rigidbody_component_factory.h"
#include "factories/rigidbody_entity_factory.h"
#include "systems/cpu_physics_collision_system.h"

namespace cpu_physics {

// Physics Layer for collision filtering
struct PhysicsLayer {
    uint32_t id;
    std::string name;
    std::vector<uint32_t> interactionLayers; // Layers this layer can interact with
};

/**
 * CPU Physics Engine - Manages ECS-based rigidbody physics with collision system
 * 
 * New architecture:
 * - Uses ECS Manager for component storage
 * - Uses Entity Factory for rigidbody creation
 * - Uses Collision System for physics simulation
 * - Maintains layer system for collision filtering
 */
class CPUPhysicsEngine {
public:
    CPUPhysicsEngine();
    ~CPUPhysicsEngine() = default;
    
    bool initialize(uint32_t maxRigidBodies = 512);
    void cleanup();
    
    // RigidBody management (ECS-style) - delegates to entity factory
    uint32_t createRigidBody(float x, float y, float z, float width, float height, float depth, float mass = 1.0f, uint32_t layer = 0);
    bool removeRigidBody(uint32_t entityId);
    RigidBodyComponent* getRigidBody(uint32_t entityId); // Legacy compatibility
    
    // Physics simulation - delegates to collision system
    void updatePhysics(float deltaTime);
    void setGravity(float x, float y, float z);
    
    // Layer system for collision filtering
    uint32_t createLayer(const std::string& name);
    bool setLayerInteraction(uint32_t layer1, uint32_t layer2, bool canInteract);
    bool canLayersInteract(uint32_t layer1, uint32_t layer2) const;
    PhysicsLayer* getLayer(uint32_t layerId);
    size_t getLayerCount() const { return layers.size(); }
    
    // ECS access for advanced usage
    std::shared_ptr<ECSManager> getECSManager() const { return ecsManager; }
    std::shared_ptr<RigidBodyEntityFactory> getEntityFactory() const { return entityFactory; }
    std::shared_ptr<CPUPhysicsCollisionSystem> getCollisionSystem() const { return collisionSystem; }
    
    // Configuration and statistics
    uint32_t getMaxRigidBodies() const { return maxRigidBodies; }
    size_t getRigidBodyCount() const;

private:
    // ECS Architecture
    std::shared_ptr<ECSManager> ecsManager;
    std::shared_ptr<RigidBodyEntityFactory> entityFactory;
    std::shared_ptr<CPUPhysicsCollisionSystem> collisionSystem;
    
    // Legacy rigidbody tracking (for compatibility)
    std::unordered_map<uint32_t, std::unique_ptr<RigidBodyComponent>> legacyRigidBodies;
    
    // Configuration
    uint32_t maxRigidBodies = 512;
    
    // Layer system
    std::unordered_map<uint32_t, PhysicsLayer> layers;
    uint32_t nextLayerId = 1;
    
    // Physics settings
    struct {
        float x = 0.0f;
        float y = -9.81f;
        float z = 0.0f;
    } gravity;
    
    // Layer interaction callback for collision system
    bool layerInteractionCallback(uint32_t layer1, uint32_t layer2);
    
    // Legacy compatibility helpers
    void updateLegacyRigidBodyData(uint32_t entityId);
    RigidBodyComponent* createLegacyRigidBodyWrapper(uint32_t entityId);
};

} // namespace cpu_physics