#include "CPUPhysicsEngine.h"
#include "../managers/logmanager/Logger.h"
#include <algorithm>
#include <cmath>

namespace cpu_physics {

CPUPhysicsEngine::CPUPhysicsEngine() {
    LOG_INFO(LogCategory::PHYSICS, "Creating CPU Physics Engine with ECS architecture");
}

bool CPUPhysicsEngine::initialize(uint32_t maxRigidBodies) {
    this->maxRigidBodies = maxRigidBodies;
    
    LOG_INFO(LogCategory::PHYSICS, "Initializing CPU Physics Engine with " + 
             std::to_string(maxRigidBodies) + " max rigidbodies");
    
    // Initialize ECS architecture
    ecsManager = std::make_shared<ECSManager>();
    entityFactory = std::make_shared<RigidBodyEntityFactory>(ecsManager);
    collisionSystem = std::make_shared<CPUPhysicsCollisionSystem>(ecsManager);
    
    // Set up layer interaction callback
    collisionSystem->setLayerInteractionCallback(
        [this](uint32_t layer1, uint32_t layer2) {
            return layerInteractionCallback(layer1, layer2);
        }
    );
    
    // Create default layer (layer 0)
    createLayer("Default");
    
    LOG_INFO(LogCategory::PHYSICS, "CPU Physics Engine initialized successfully");
    return true;
}

void CPUPhysicsEngine::cleanup() {
    LOG_INFO(LogCategory::PHYSICS, "Cleaning up CPU Physics Engine");
    
    // Clear legacy rigidbody wrappers
    legacyRigidBodies.clear();
    
    // ECS components will be cleaned up automatically when shared_ptrs are destroyed
    collisionSystem.reset();
    entityFactory.reset();
    ecsManager.reset();
    
    layers.clear();
}

uint32_t CPUPhysicsEngine::createRigidBody(float x, float y, float z, float width, float height, float depth, float mass, uint32_t layer) {
    if (!entityFactory) {
        LOG_ERROR(LogCategory::RIGIDBODY, "Entity factory not initialized");
        return 0;
    }
    
    // Use entity factory to create the rigidbody
    uint32_t entityId = entityFactory->createRigidBody(x, y, z, width, height, depth, mass, layer);
    
    if (entityId != 0) {
        // Create legacy wrapper for compatibility
        createLegacyRigidBodyWrapper(entityId);
        
        LOG_INFO(LogCategory::RIGIDBODY, 
            "Created rigidbody " + std::to_string(entityId) + 
            " at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")" +
            " with dimensions (" + std::to_string(width) + ", " + std::to_string(height) + ", " + std::to_string(depth) + ")");
    }
    
    return entityId;
}

bool CPUPhysicsEngine::removeRigidBody(uint32_t entityId) {
    if (!entityFactory) {
        return false;
    }
    
    // Remove legacy wrapper
    legacyRigidBodies.erase(entityId);
    
    // Remove from ECS
    return entityFactory->destroyRigidBody(entityId);
}

RigidBodyComponent* CPUPhysicsEngine::getRigidBody(uint32_t entityId) {
    auto it = legacyRigidBodies.find(entityId);
    if (it == legacyRigidBodies.end()) {
        return nullptr;
    }
    
    // Update legacy wrapper with current ECS data
    updateLegacyRigidBodyData(entityId);
    
    return it->second.get();
}

void CPUPhysicsEngine::updatePhysics(float deltaTime) {
    if (!collisionSystem) {
        LOG_WARN(LogCategory::PHYSICS, "Collision system not initialized");
        return;
    }
    
    // Delegate to collision system
    collisionSystem->update(deltaTime);
    
    // Update legacy rigidbody wrappers
    for (const auto& [entityId, wrapper] : legacyRigidBodies) {
        updateLegacyRigidBodyData(entityId);
    }
}

void CPUPhysicsEngine::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    if (collisionSystem) {
        collisionSystem->setGravity(x, y, z);
    }
    
    LOG_INFO(LogCategory::PHYSICS, 
        "CPU Physics gravity set to (" + 
        std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

uint32_t CPUPhysicsEngine::createLayer(const std::string& name) {
    PhysicsLayer layer;
    layer.id = nextLayerId++;
    layer.name = name;
    
    layers[layer.id] = layer;
    
    LOG_INFO(LogCategory::PHYSICS, "Created physics layer '" + name + "' with ID " + std::to_string(layer.id));
    return layer.id;
}

bool CPUPhysicsEngine::setLayerInteraction(uint32_t layer1, uint32_t layer2, bool canInteract) {
    auto it1 = layers.find(layer1);
    auto it2 = layers.find(layer2);
    
    if (it1 == layers.end() || it2 == layers.end()) {
        return false;
    }
    
    if (canInteract) {
        // Add to interaction lists if not already present
        auto& interactions1 = it1->second.interactionLayers;
        auto& interactions2 = it2->second.interactionLayers;
        
        if (std::find(interactions1.begin(), interactions1.end(), layer2) == interactions1.end()) {
            interactions1.push_back(layer2);
        }
        
        if (layer1 != layer2) { // Don't duplicate self-interactions
            if (std::find(interactions2.begin(), interactions2.end(), layer1) == interactions2.end()) {
                interactions2.push_back(layer1);
            }
        }
    } else {
        // Remove from interaction lists
        auto& interactions1 = it1->second.interactionLayers;
        auto& interactions2 = it2->second.interactionLayers;
        
        interactions1.erase(std::remove(interactions1.begin(), interactions1.end(), layer2), interactions1.end());
        
        if (layer1 != layer2) {
            interactions2.erase(std::remove(interactions2.begin(), interactions2.end(), layer1), interactions2.end());
        }
    }
    
    LOG_INFO(LogCategory::PHYSICS, 
        "Set layer interaction between " + std::to_string(layer1) + 
        " and " + std::to_string(layer2) + ": " + (canInteract ? "enabled" : "disabled"));
    
    return true;
}

bool CPUPhysicsEngine::canLayersInteract(uint32_t layer1, uint32_t layer2) const {
    auto it = layers.find(layer1);
    if (it == layers.end()) {
        return false;
    }
    
    const auto& interactions = it->second.interactionLayers;
    return std::find(interactions.begin(), interactions.end(), layer2) != interactions.end();
}

PhysicsLayer* CPUPhysicsEngine::getLayer(uint32_t layerId) {
    auto it = layers.find(layerId);
    return (it != layers.end()) ? &it->second : nullptr;
}

size_t CPUPhysicsEngine::getRigidBodyCount() const {
    if (!entityFactory) {
        return 0;
    }
    return entityFactory->getRigidBodyCount();
}

bool CPUPhysicsEngine::layerInteractionCallback(uint32_t layer1, uint32_t layer2) {
    return canLayersInteract(layer1, layer2);
}

void CPUPhysicsEngine::updateLegacyRigidBodyData(uint32_t entityId) {
    auto it = legacyRigidBodies.find(entityId);
    if (it == legacyRigidBodies.end() || !ecsManager) {
        return;
    }
    
    auto* transform = ecsManager->getComponent<TransformComponent>(entityId);
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    auto* collider = ecsManager->getComponent<BoxColliderComponent>(entityId);
    
    if (transform && physics && collider) {
        auto& wrapper = *it->second;
        wrapper.transform = *transform;
        wrapper.physics = *physics;
        wrapper.collider = *collider;
    }
}

RigidBodyComponent* CPUPhysicsEngine::createLegacyRigidBodyWrapper(uint32_t entityId) {
    if (!ecsManager) {
        return nullptr;
    }
    
    auto wrapper = std::make_unique<RigidBodyComponent>();
    wrapper->entityId = entityId;
    wrapper->hasCollider = true;
    wrapper->layer = 0; // TODO: Add layer component support
    
    // Initialize with current ECS data
    auto* transform = ecsManager->getComponent<TransformComponent>(entityId);
    auto* physics = ecsManager->getComponent<PhysicsComponent>(entityId);
    auto* collider = ecsManager->getComponent<BoxColliderComponent>(entityId);
    
    if (transform && physics && collider) {
        wrapper->transform = *transform;
        wrapper->physics = *physics;
        wrapper->collider = *collider;
    }
    
    RigidBodyComponent* result = wrapper.get();
    legacyRigidBodies[entityId] = std::move(wrapper);
    
    return result;
}

} // namespace cpu_physics