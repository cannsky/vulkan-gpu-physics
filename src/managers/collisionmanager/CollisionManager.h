#pragma once

#include "../BaseManager.h"
#include "Contact.h"
#include <vector>
#include <memory>

// Forward declarations
struct RigidBody;
class RigidBodySystem;
class BroadPhaseWorker;
class DetectCollisionWorker;
class ContactResolverWorker;
class GPUBufferWorker;

/**
 * Manager for collision detection and resolution.
 * Provides singleton access to collision operations with modular workers.
 */
class CollisionManager : public BaseManager {
public:
    static CollisionManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Collision operations
    void updateCollisions(float deltaTime);
    void detectCollisions();
    void resolveContacts(float deltaTime);
    
    // Broad phase operations
    void updateBroadPhase(const std::vector<RigidBody>& rigidBodies);
    
    // Statistics
    uint32_t getContactCount() const;
    uint32_t getCollisionPairCount() const;
    
    // GPU operations
    void uploadContactsToGPU();
    void downloadContactsFromGPU();

private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    
    bool initialized = false;
    uint32_t maxContacts = 1024;
    uint32_t contactCount = 0;
    
    // Worker components
    std::unique_ptr<BroadPhaseWorker> broadPhaseWorker;
    std::unique_ptr<DetectCollisionWorker> detectCollisionWorker;
    std::unique_ptr<ContactResolverWorker> contactResolverWorker;
    std::unique_ptr<GPUBufferWorker> gpuBufferWorker;
    
    // Data storage
    std::vector<Contact> contacts;
    std::vector<CollisionPair> collisionPairs;
};