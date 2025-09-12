#include "CollisionSystem.h"
#include "../physics/components/BufferManager.h"
#include "../physics/VulkanContext.h"
#include "../rigidbody/RigidBodySystem.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <algorithm>

CollisionSystem::CollisionSystem(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager)
    : vulkanContext(context), bufferManager(bufferManager), maxContacts(0), contactCount(0) {
}

CollisionSystem::~CollisionSystem() {
    cleanup();
}

bool CollisionSystem::initialize(uint32_t maxContacts) {
    this->maxContacts = maxContacts;
    contacts.reserve(maxContacts);
    
    std::cout << "CollisionSystem initialized with capacity for " << maxContacts << " contacts" << std::endl;
    return true;
}

void CollisionSystem::cleanup() {
    contacts.clear();
    collisionPairs.clear();
    
    // Clean up GPU resources
    if (contactBuffer != VK_NULL_HANDLE) {
        // Cleanup would be handled by BufferManager or VulkanContext
    }
}

void CollisionSystem::updateBroadPhase(const std::vector<RigidBody>& rigidBodies) {
    // Simple O(n²) broad phase for now
    // In production, use spatial partitioning (BVH, grid, etc.)
    
    collisionPairs.clear();
    
    for (size_t i = 0; i < rigidBodies.size(); ++i) {
        for (size_t j = i + 1; j < rigidBodies.size(); ++j) {
            const RigidBody& bodyA = rigidBodies[i];
            const RigidBody& bodyB = rigidBodies[j];
            
            // Skip if both bodies are static
            if (bodyA.isStatic && bodyB.isStatic) {
                continue;
            }
            
            // Simple AABB check for broad phase
            float dx = bodyA.position[0] - bodyB.position[0];
            float dy = bodyA.position[1] - bodyB.position[1];
            float dz = bodyA.position[2] - bodyB.position[2];
            float distSq = dx * dx + dy * dy + dz * dz;
            
            // Conservative bounding sphere check
            float maxRadiusA = std::max({bodyA.shapeData[0], bodyA.shapeData[1], bodyA.shapeData[2]});
            float maxRadiusB = std::max({bodyB.shapeData[0], bodyB.shapeData[1], bodyB.shapeData[2]});
            float maxDist = maxRadiusA + maxRadiusB;
            
            if (distSq <= maxDist * maxDist) {
                CollisionPair pair;
                pair.bodyIdA = static_cast<uint32_t>(i);
                pair.bodyIdB = static_cast<uint32_t>(j);
                pair.isActive = 1;
                collisionPairs.push_back(pair);
            }
        }
    }
}

void CollisionSystem::detectCollisions(std::shared_ptr<RigidBodySystem> rigidBodySystem) {
    contacts.clear();
    contactCount = 0;
    
    // For each collision pair, perform narrow phase detection
    for (const auto& pair : collisionPairs) {
        RigidBody* bodyA = rigidBodySystem->getRigidBody(pair.bodyIdA);
        RigidBody* bodyB = rigidBodySystem->getRigidBody(pair.bodyIdB);
        
        if (!bodyA || !bodyB) continue;
        
        Contact contact;
        bool hasCollision = false;
        
        // Dispatch to appropriate collision detection function
        if (bodyA->shapeType == static_cast<uint32_t>(RigidBodyShape::SPHERE) &&
            bodyB->shapeType == static_cast<uint32_t>(RigidBodyShape::SPHERE)) {
            hasCollision = sphereVsSphere(*bodyA, *bodyB, contact);
        }
        else if ((bodyA->shapeType == static_cast<uint32_t>(RigidBodyShape::SPHERE) &&
                  bodyB->shapeType == static_cast<uint32_t>(RigidBodyShape::BOX)) ||
                 (bodyA->shapeType == static_cast<uint32_t>(RigidBodyShape::BOX) &&
                  bodyB->shapeType == static_cast<uint32_t>(RigidBodyShape::SPHERE))) {
            // Ensure sphere is always bodyA for consistency
            if (bodyA->shapeType == static_cast<uint32_t>(RigidBodyShape::BOX)) {
                hasCollision = sphereVsBox(*bodyB, *bodyA, contact);
                // Flip the contact normal since we swapped bodies
                contact.normal[0] = -contact.normal[0];
                contact.normal[1] = -contact.normal[1];
                contact.normal[2] = -contact.normal[2];
                contact.bodyIdA = pair.bodyIdB;
                contact.bodyIdB = pair.bodyIdA;
            } else {
                hasCollision = sphereVsBox(*bodyA, *bodyB, contact);
                contact.bodyIdA = pair.bodyIdA;
                contact.bodyIdB = pair.bodyIdB;
            }
        }
        else if (bodyA->shapeType == static_cast<uint32_t>(RigidBodyShape::BOX) &&
                 bodyB->shapeType == static_cast<uint32_t>(RigidBodyShape::BOX)) {
            hasCollision = boxVsBox(*bodyA, *bodyB, contact);
        }
        
        if (hasCollision && contactCount < maxContacts) {
            contact.bodyIdA = pair.bodyIdA;
            contact.bodyIdB = pair.bodyIdB;
            contact.restitution = combinedRestitution(bodyA->restitution, bodyB->restitution);
            contact.friction = combinedFriction(bodyA->friction, bodyB->friction);
            
            contacts.push_back(contact);
            contactCount++;
        }
    }
}

void CollisionSystem::resolveContacts(float deltaTime) {
    // Simple impulse-based resolution
    // In production, this would run on GPU
    
    for (const auto& contact : contacts) {
        // This is a simplified version - full resolution would handle
        // angular momentum, friction, and iterative solving
        std::cout << "Resolving contact between bodies " 
                  << contact.bodyIdA << " and " << contact.bodyIdB 
                  << " with penetration " << contact.penetration << std::endl;
    }
}

void CollisionSystem::uploadContactsToGPU() {
    // Upload contact data to GPU buffers for compute shader processing
    std::cout << "Uploading " << contactCount << " contacts to GPU" << std::endl;
}

void CollisionSystem::downloadContactsFromGPU() {
    // Download resolved contact data from GPU
    std::cout << "Downloading contacts from GPU" << std::endl;
}

bool CollisionSystem::sphereVsSphere(const RigidBody& bodyA, const RigidBody& bodyB, Contact& contact) {
    float dx = bodyA.position[0] - bodyB.position[0];
    float dy = bodyA.position[1] - bodyB.position[1];
    float dz = bodyA.position[2] - bodyB.position[2];
    float distSq = dx * dx + dy * dy + dz * dz;
    
    float radiusSum = bodyA.shapeData[0] + bodyB.shapeData[0];
    
    if (distSq >= radiusSum * radiusSum) {
        return false; // No collision
    }
    
    float dist = std::sqrt(distSq);
    
    // Contact normal (from A to B)
    if (dist > 0.0f) {
        contact.normal[0] = dx / dist;
        contact.normal[1] = dy / dist;
        contact.normal[2] = dz / dist;
    } else {
        // Spheres are exactly overlapping - use arbitrary normal
        contact.normal[0] = 1.0f;
        contact.normal[1] = 0.0f;
        contact.normal[2] = 0.0f;
    }
    
    // Contact position (midpoint)
    contact.position[0] = bodyA.position[0] - contact.normal[0] * bodyA.shapeData[0];
    contact.position[1] = bodyA.position[1] - contact.normal[1] * bodyA.shapeData[0];
    contact.position[2] = bodyA.position[2] - contact.normal[2] * bodyA.shapeData[0];
    
    // Penetration depth
    contact.penetration = radiusSum - dist;
    
    return true;
}

bool CollisionSystem::sphereVsBox(const RigidBody& sphere, const RigidBody& box, Contact& contact) {
    // Simplified sphere vs box collision detection
    // For a complete implementation, we'd need to handle rotated boxes
    
    float halfExtents[3] = {box.shapeData[0], box.shapeData[1], box.shapeData[2]};
    
    // Find closest point on box to sphere center
    float closest[3];
    for (int i = 0; i < 3; ++i) {
        closest[i] = std::max(box.position[i] - halfExtents[i],
                             std::min(sphere.position[i], box.position[i] + halfExtents[i]));
    }
    
    // Calculate distance from sphere center to closest point
    float dx = sphere.position[0] - closest[0];
    float dy = sphere.position[1] - closest[1];
    float dz = sphere.position[2] - closest[2];
    float distSq = dx * dx + dy * dy + dz * dz;
    
    float radius = sphere.shapeData[0];
    
    if (distSq >= radius * radius) {
        return false; // No collision
    }
    
    float dist = std::sqrt(distSq);
    
    // Contact normal (from sphere to box)
    if (dist > 0.0f) {
        contact.normal[0] = dx / dist;
        contact.normal[1] = dy / dist;
        contact.normal[2] = dz / dist;
    } else {
        // Sphere center is inside box - find separation direction
        // Use direction to nearest face
        float minDist = FLT_MAX;
        int axis = 0;
        for (int i = 0; i < 3; ++i) {
            float distToFace = halfExtents[i] - std::abs(sphere.position[i] - box.position[i]);
            if (distToFace < minDist) {
                minDist = distToFace;
                axis = i;
            }
        }
        
        contact.normal[0] = contact.normal[1] = contact.normal[2] = 0.0f;
        contact.normal[axis] = (sphere.position[axis] > box.position[axis]) ? 1.0f : -1.0f;
    }
    
    // Contact position
    contact.position[0] = closest[0];
    contact.position[1] = closest[1];
    contact.position[2] = closest[2];
    
    // Penetration depth
    contact.penetration = radius - dist;
    
    return true;
}

bool CollisionSystem::boxVsBox(const RigidBody& bodyA, const RigidBody& bodyB, Contact& contact) {
    // Simplified box vs box collision detection
    // This is a placeholder - full OBB vs OBB collision is complex
    
    // For now, just check AABB overlap
    float halfExtentsA[3] = {bodyA.shapeData[0], bodyA.shapeData[1], bodyA.shapeData[2]};
    float halfExtentsB[3] = {bodyB.shapeData[0], bodyB.shapeData[1], bodyB.shapeData[2]};
    
    for (int i = 0; i < 3; ++i) {
        float overlap = (halfExtentsA[i] + halfExtentsB[i]) - std::abs(bodyA.position[i] - bodyB.position[i]);
        if (overlap <= 0.0f) {
            return false; // No collision
        }
    }
    
    // Find minimum overlap axis for separation
    float minOverlap = FLT_MAX;
    int separationAxis = 0;
    
    for (int i = 0; i < 3; ++i) {
        float overlap = (halfExtentsA[i] + halfExtentsB[i]) - std::abs(bodyA.position[i] - bodyB.position[i]);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            separationAxis = i;
        }
    }
    
    // Contact normal
    contact.normal[0] = contact.normal[1] = contact.normal[2] = 0.0f;
    contact.normal[separationAxis] = (bodyA.position[separationAxis] > bodyB.position[separationAxis]) ? 1.0f : -1.0f;
    
    // Contact position (approximate)
    contact.position[0] = (bodyA.position[0] + bodyB.position[0]) * 0.5f;
    contact.position[1] = (bodyA.position[1] + bodyB.position[1]) * 0.5f;
    contact.position[2] = (bodyA.position[2] + bodyB.position[2]) * 0.5f;
    
    // Penetration depth
    contact.penetration = minOverlap;
    
    return true;
}

float CollisionSystem::combinedRestitution(float restA, float restB) const {
    // Average the restitution values
    return (restA + restB) * 0.5f;
}

float CollisionSystem::combinedFriction(float fricA, float fricB) const {
    // Use geometric mean for friction
    return std::sqrt(fricA * fricB);
}