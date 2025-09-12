#pragma once

#include "Contact.h"
#include "../rigidbody/RigidBody.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class VulkanContext;
class BufferManager;
class RigidBodySystem;

class CollisionSystem {
public:
    CollisionSystem(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager);
    ~CollisionSystem();
    
    bool initialize(uint32_t maxContacts = 1024);
    void cleanup();
    
    // Broad phase collision detection
    void updateBroadPhase(const std::vector<RigidBody>& rigidBodies);
    
    // Narrow phase collision detection (runs on GPU)
    void detectCollisions(std::shared_ptr<RigidBodySystem> rigidBodySystem);
    
    // Contact resolution (runs on GPU)
    void resolveContacts(float deltaTime);
    
    // Get collision statistics
    uint32_t getContactCount() const { return contactCount; }
    uint32_t getCollisionPairCount() const { return static_cast<uint32_t>(collisionPairs.size()); }
    
    // GPU buffer management
    void uploadContactsToGPU();
    void downloadContactsFromGPU();

private:
    // CPU collision detection functions (for debugging/validation)
    bool sphereVsSphere(const RigidBody& bodyA, const RigidBody& bodyB, Contact& contact);
    bool sphereVsBox(const RigidBody& sphere, const RigidBody& box, Contact& contact);
    bool boxVsBox(const RigidBody& bodyA, const RigidBody& bodyB, Contact& contact);
    
    // Utility functions
    float combinedRestitution(float restA, float restB) const;
    float combinedFriction(float fricA, float fricB) const;
    
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<BufferManager> bufferManager;
    
    std::vector<Contact> contacts;
    std::vector<CollisionPair> collisionPairs;
    uint32_t maxContacts;
    uint32_t contactCount;
    
    // GPU compute resources
    VkBuffer contactBuffer = VK_NULL_HANDLE;
    VkDeviceMemory contactBufferMemory = VK_NULL_HANDLE;
    VkBuffer collisionPairBuffer = VK_NULL_HANDLE;
    VkDeviceMemory collisionPairBufferMemory = VK_NULL_HANDLE;
};