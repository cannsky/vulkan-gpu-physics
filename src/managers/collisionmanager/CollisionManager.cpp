#include "CollisionManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "workers/BroadPhaseWorker.h"
#include "workers/DetectCollisionWorker.h"
#include "workers/ContactResolverWorker.h"
#include "workers/GPUBufferWorker.h"

CollisionManager& CollisionManager::getInstance() {
    static CollisionManager instance;
    return instance;
}

bool CollisionManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Initialize workers
        broadPhaseWorker = std::make_unique<BroadPhaseWorker>();
        detectCollisionWorker = std::make_unique<DetectCollisionWorker>();
        contactResolverWorker = std::make_unique<ContactResolverWorker>();
        
        // Initialize GPU buffer worker (commented out to avoid Vulkan dependencies for now)
        // auto& vulkanManager = VulkanManager::getInstance();
        // if (vulkanManager.isInitialized()) {
        //     gpuBufferWorker = std::make_unique<GPUBufferWorker>(nullptr, nullptr);
        //     gpuBufferWorker->initialize(maxContacts);
        // }
        
        // Reserve memory for contacts and collision pairs
        contacts.reserve(maxContacts);
        collisionPairs.reserve(maxContacts);
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void CollisionManager::cleanup() {
    if (gpuBufferWorker) {
        gpuBufferWorker->cleanup();
        gpuBufferWorker.reset();
    }
    
    contactResolverWorker.reset();
    detectCollisionWorker.reset();
    broadPhaseWorker.reset();
    
    contacts.clear();
    collisionPairs.clear();
    
    initialized = false;
}

bool CollisionManager::isInitialized() const {
    return initialized;
}

void CollisionManager::updateCollisions(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    // Note: detectCollisions now requires RigidBodyWorker to be passed from caller
    resolveContacts(deltaTime);
}

void CollisionManager::updateBroadPhase(const std::vector<RigidBody>& rigidBodies) {
    if (!initialized || !broadPhaseWorker) {
        return;
    }
    
    broadPhaseWorker->updateBroadPhase(rigidBodies, collisionPairs);
}

void CollisionManager::detectCollisions(std::shared_ptr<RigidBodyWorker> rigidBodyWorker) {
    if (!initialized || !detectCollisionWorker || !rigidBodyWorker) {
        return;
    }
    
    detectCollisionWorker->detectCollisions(collisionPairs, rigidBodyWorker, contacts, maxContacts, contactCount);
}

void CollisionManager::resolveContacts(float deltaTime) {
    if (!initialized || !contactResolverWorker) {
        return;
    }
    
    contactResolverWorker->resolveContacts(contacts, deltaTime);
}

uint32_t CollisionManager::getContactCount() const {
    return contactCount;
}

uint32_t CollisionManager::getCollisionPairCount() const {
    return static_cast<uint32_t>(collisionPairs.size());
}

void CollisionManager::uploadContactsToGPU() {
    if (!gpuBufferWorker) {
        return;
    }
    gpuBufferWorker->uploadContactsToGPU(contactCount);
}

void CollisionManager::downloadContactsFromGPU() {
    if (!gpuBufferWorker) {
        return;
    }
    gpuBufferWorker->downloadContactsFromGPU();
}