#include "CPUPhysicsSystem.h"
#include "../managers/logmanager/Logger.h"
#include <algorithm>
#include <cmath>

namespace cpu_physics {

CPUPhysicsSystem::CPUPhysicsSystem() {
    LOG_INFO(LogCategory::PHYSICS, "Creating CPU Physics System with ECS architecture");
}

bool CPUPhysicsSystem::initialize(uint32_t maxRigidBodies) {
    this->maxRigidBodies = maxRigidBodies;
    
    LOG_INFO(LogCategory::PHYSICS, "Initializing CPU Physics System with " + 
             std::to_string(maxRigidBodies) + " max rigidbodies");
    
    // Create default layer (layer 0)
    createLayer("Default");
    
    LOG_INFO(LogCategory::PHYSICS, "CPU Physics System initialized successfully");
    return true;
}

void CPUPhysicsSystem::cleanup() {
    rigidBodies.clear();
    layers.clear();
    nextEntityId = 1;
    nextLayerId = 1;
    
    LOG_INFO(LogCategory::PHYSICS, "CPU Physics System cleanup complete");
}

uint32_t CPUPhysicsSystem::createRigidBody(float x, float y, float z, float width, float height, float depth, float mass, uint32_t layer) {
    if (rigidBodies.size() >= maxRigidBodies) {
        LOG_ERROR(LogCategory::RIGIDBODY, "Cannot create rigidbody: maximum capacity reached");
        return 0;
    }
    
    uint32_t entityId = nextEntityId++;
    
    RigidBodyComponent& body = rigidBodies[entityId];
    body.entityId = entityId;
    
    // Initialize transform
    body.transform.position[0] = x;
    body.transform.position[1] = y;
    body.transform.position[2] = z;
    body.transform.rotation[0] = 1.0f; // w
    body.transform.rotation[1] = 0.0f; // x
    body.transform.rotation[2] = 0.0f; // y
    body.transform.rotation[3] = 0.0f; // z
    body.transform.scale[0] = 1.0f;
    body.transform.scale[1] = 1.0f;
    body.transform.scale[2] = 1.0f;
    
    // Initialize physics
    body.physics.mass = mass;
    body.physics.invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    body.physics.isStatic = (mass <= 0.0f);
    body.physics.restitution = 0.5f;
    body.physics.friction = 0.3f;
    body.physics.useGravity = !body.physics.isStatic;
    
    // Initialize box collider (only supported collider type)
    body.collider.width = width;
    body.collider.height = height;
    body.collider.depth = depth;
    body.collider.enabled = true;
    body.hasCollider = true;
    
    // Set layer
    body.layer = layer;
    
    LOG_INFO(LogCategory::RIGIDBODY, "Created rigidbody entity " + std::to_string(entityId) + 
             " at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + 
             ") with box collider (" + std::to_string(width) + ", " + std::to_string(height) + ", " + std::to_string(depth) + ")");
    
    return entityId;
}

bool CPUPhysicsSystem::removeRigidBody(uint32_t entityId) {
    auto it = rigidBodies.find(entityId);
    if (it == rigidBodies.end()) {
        return false;
    }
    
    rigidBodies.erase(it);
    LOG_INFO(LogCategory::RIGIDBODY, "Removed rigidbody entity " + std::to_string(entityId));
    return true;
}

RigidBodyComponent* CPUPhysicsSystem::getRigidBody(uint32_t entityId) {
    auto it = rigidBodies.find(entityId);
    if (it == rigidBodies.end()) {
        return nullptr;
    }
    
    return &it->second;
}

void CPUPhysicsSystem::updatePhysics(float deltaTime) {
    // Integrate physics for all rigidbodies
    for (auto& [entityId, body] : rigidBodies) {
        integratePhysics(body, deltaTime);
    }
    
    // Detect and resolve collisions
    detectAndResolveCollisions(deltaTime);
}

void CPUPhysicsSystem::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    LOG_INFO(LogCategory::PHYSICS, "CPU Physics gravity set to (" + 
             std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

uint32_t CPUPhysicsSystem::createLayer(const std::string& name) {
    uint32_t layerId = nextLayerId++;
    
    PhysicsLayer& layer = layers[layerId];
    layer.id = layerId;
    layer.name = name;
    
    // By default, layers can interact with themselves
    layer.interactionLayers.push_back(layerId);
    
    LOG_INFO(LogCategory::PHYSICS, "Created physics layer '" + name + "' with ID " + std::to_string(layerId));
    return layerId;
}

bool CPUPhysicsSystem::setLayerInteraction(uint32_t layer1, uint32_t layer2, bool canInteract) {
    auto it1 = layers.find(layer1);
    auto it2 = layers.find(layer2);
    
    if (it1 == layers.end() || it2 == layers.end()) {
        return false;
    }
    
    PhysicsLayer& l1 = it1->second;
    PhysicsLayer& l2 = it2->second;
    
    auto& interactions1 = l1.interactionLayers;
    auto& interactions2 = l2.interactionLayers;
    
    if (canInteract) {
        // Add interaction if not already present
        if (std::find(interactions1.begin(), interactions1.end(), layer2) == interactions1.end()) {
            interactions1.push_back(layer2);
        }
        if (std::find(interactions2.begin(), interactions2.end(), layer1) == interactions2.end()) {
            interactions2.push_back(layer1);
        }
    } else {
        // Remove interaction
        interactions1.erase(std::remove(interactions1.begin(), interactions1.end(), layer2), interactions1.end());
        interactions2.erase(std::remove(interactions2.begin(), interactions2.end(), layer1), interactions2.end());
    }
    
    return true;
}

bool CPUPhysicsSystem::canLayersInteract(uint32_t layer1, uint32_t layer2) const {
    auto it1 = layers.find(layer1);
    if (it1 == layers.end()) {
        return false;
    }
    
    const auto& interactions = it1->second.interactionLayers;
    return std::find(interactions.begin(), interactions.end(), layer2) != interactions.end();
}

PhysicsLayer* CPUPhysicsSystem::getLayer(uint32_t layerId) {
    auto it = layers.find(layerId);
    if (it == layers.end()) {
        return nullptr;
    }
    
    return &it->second;
}

void CPUPhysicsSystem::integratePhysics(RigidBodyComponent& body, float deltaTime) {
    if (body.physics.isStatic) {
        return;
    }
    
    // Apply gravity
    if (body.physics.useGravity && body.physics.invMass > 0.0f) {
        body.physics.velocity[0] += gravity.x * deltaTime;
        body.physics.velocity[1] += gravity.y * deltaTime;
        body.physics.velocity[2] += gravity.z * deltaTime;
    }
    
    // Integrate position
    body.transform.position[0] += body.physics.velocity[0] * deltaTime;
    body.transform.position[1] += body.physics.velocity[1] * deltaTime;
    body.transform.position[2] += body.physics.velocity[2] * deltaTime;
    
    // Apply basic damping to prevent infinite acceleration
    const float damping = 0.999f;
    body.physics.velocity[0] *= damping;
    body.physics.velocity[1] *= damping;
    body.physics.velocity[2] *= damping;
}

void CPUPhysicsSystem::detectAndResolveCollisions(float deltaTime) {
    // Simple O(n²) collision detection - suitable for basic use cases
    for (auto it1 = rigidBodies.begin(); it1 != rigidBodies.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != rigidBodies.end(); ++it2) {
            RigidBodyComponent& body1 = it1->second;
            RigidBodyComponent& body2 = it2->second;
            
            // Check if layers can interact
            if (!canLayersInteract(body1.layer, body2.layer)) {
                continue;
            }
            
            // Check if both have colliders enabled
            if (!body1.hasCollider || !body2.hasCollider || !body1.collider.enabled || !body2.collider.enabled) {
                continue;
            }
            
            // Check for collision (only box-box for now)
            if (checkBoxBoxCollision(body1, body2)) {
                resolveCollision(body1, body2);
            }
        }
    }
}

bool CPUPhysicsSystem::checkBoxBoxCollision(const RigidBodyComponent& body1, const RigidBodyComponent& body2) {
    // AABB collision detection
    float minX1 = body1.transform.position[0] - body1.collider.width * 0.5f;
    float maxX1 = body1.transform.position[0] + body1.collider.width * 0.5f;
    float minY1 = body1.transform.position[1] - body1.collider.height * 0.5f;
    float maxY1 = body1.transform.position[1] + body1.collider.height * 0.5f;
    float minZ1 = body1.transform.position[2] - body1.collider.depth * 0.5f;
    float maxZ1 = body1.transform.position[2] + body1.collider.depth * 0.5f;
    
    float minX2 = body2.transform.position[0] - body2.collider.width * 0.5f;
    float maxX2 = body2.transform.position[0] + body2.collider.width * 0.5f;
    float minY2 = body2.transform.position[1] - body2.collider.height * 0.5f;
    float maxY2 = body2.transform.position[1] + body2.collider.height * 0.5f;
    float minZ2 = body2.transform.position[2] - body2.collider.depth * 0.5f;
    float maxZ2 = body2.transform.position[2] + body2.collider.depth * 0.5f;
    
    return (minX1 <= maxX2 && maxX1 >= minX2) &&
           (minY1 <= maxY2 && maxY1 >= minY2) &&
           (minZ1 <= maxZ2 && maxZ1 >= minZ2);
}

void CPUPhysicsSystem::resolveCollision(RigidBodyComponent& body1, RigidBodyComponent& body2) {
    // Simple collision response - separate the bodies and apply impulse
    
    // Calculate separation vector
    float dx = body1.transform.position[0] - body2.transform.position[0];
    float dy = body1.transform.position[1] - body2.transform.position[1];
    float dz = body1.transform.position[2] - body2.transform.position[2];
    
    // Calculate penetration depth for each axis
    float penetrationX = (body1.collider.width + body2.collider.width) * 0.5f - std::abs(dx);
    float penetrationY = (body1.collider.height + body2.collider.height) * 0.5f - std::abs(dy);
    float penetrationZ = (body1.collider.depth + body2.collider.depth) * 0.5f - std::abs(dz);
    
    // Find the axis with minimum penetration (collision normal)
    float minPenetration = std::min({penetrationX, penetrationY, penetrationZ});
    
    float normalX = 0.0f, normalY = 0.0f, normalZ = 0.0f;
    float separation = minPenetration;
    
    if (minPenetration == penetrationX) {
        normalX = (dx > 0) ? 1.0f : -1.0f;
    } else if (minPenetration == penetrationY) {
        normalY = (dy > 0) ? 1.0f : -1.0f;
    } else {
        normalZ = (dz > 0) ? 1.0f : -1.0f;
    }
    
    // Separate the bodies
    float totalInvMass = body1.physics.invMass + body2.physics.invMass;
    if (totalInvMass > 0.0f) {
        float ratio1 = body1.physics.invMass / totalInvMass;
        float ratio2 = body2.physics.invMass / totalInvMass;
        
        body1.transform.position[0] += normalX * separation * ratio1;
        body1.transform.position[1] += normalY * separation * ratio1;
        body1.transform.position[2] += normalZ * separation * ratio1;
        
        body2.transform.position[0] -= normalX * separation * ratio2;
        body2.transform.position[1] -= normalY * separation * ratio2;
        body2.transform.position[2] -= normalZ * separation * ratio2;
        
        // Apply impulse based on restitution
        float restitution = std::min(body1.physics.restitution, body2.physics.restitution);
        
        float relativeVelX = body1.physics.velocity[0] - body2.physics.velocity[0];
        float relativeVelY = body1.physics.velocity[1] - body2.physics.velocity[1];
        float relativeVelZ = body1.physics.velocity[2] - body2.physics.velocity[2];
        
        float contactVel = relativeVelX * normalX + relativeVelY * normalY + relativeVelZ * normalZ;
        
        if (contactVel > 0) return; // Objects are separating
        
        float impulse = -(1 + restitution) * contactVel / totalInvMass;
        
        body1.physics.velocity[0] += impulse * normalX * body1.physics.invMass;
        body1.physics.velocity[1] += impulse * normalY * body1.physics.invMass;
        body1.physics.velocity[2] += impulse * normalZ * body1.physics.invMass;
        
        body2.physics.velocity[0] -= impulse * normalX * body2.physics.invMass;
        body2.physics.velocity[1] -= impulse * normalY * body2.physics.invMass;
        body2.physics.velocity[2] -= impulse * normalZ * body2.physics.invMass;
    }
}

} // namespace cpu_physics