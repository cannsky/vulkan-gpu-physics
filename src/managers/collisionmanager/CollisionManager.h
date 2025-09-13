#pragma once

#include "../BaseManager.h"
#include "../../collision/Contact.h"
#include <vector>
#include <memory>

// Forward declarations
class CollisionSystem;
class RigidBodySystem;

/**
 * Manager for collision detection and resolution.
 * Provides singleton access to collision operations.
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
    std::shared_ptr<CollisionSystem> collisionSystem;
};