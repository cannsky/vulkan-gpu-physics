#include "BroadPhaseWorker.h"
#include <algorithm>
#include <cmath>

void BroadPhaseWorker::updateBroadPhase(const std::vector<RigidBody>& rigidBodies, std::vector<CollisionPair>& collisionPairs) {
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
            
            if (isAABBOverlapping(bodyA, bodyB)) {
                CollisionPair pair;
                pair.bodyIdA = static_cast<uint32_t>(i);
                pair.bodyIdB = static_cast<uint32_t>(j);
                pair.isActive = 1;
                collisionPairs.push_back(pair);
            }
        }
    }
}

bool BroadPhaseWorker::isAABBOverlapping(const RigidBody& bodyA, const RigidBody& bodyB) const {
    // Simple AABB check for broad phase
    float dx = bodyA.position[0] - bodyB.position[0];
    float dy = bodyA.position[1] - bodyB.position[1];
    float dz = bodyA.position[2] - bodyB.position[2];
    float distSq = dx * dx + dy * dy + dz * dz;
    
    // Conservative bounding sphere check
    float maxRadiusA = getMaxRadius(bodyA);
    float maxRadiusB = getMaxRadius(bodyB);
    float maxDist = maxRadiusA + maxRadiusB;
    
    return distSq <= maxDist * maxDist;
}

float BroadPhaseWorker::getMaxRadius(const RigidBody& body) const {
    return std::max({body.shapeData[0], body.shapeData[1], body.shapeData[2]});
}