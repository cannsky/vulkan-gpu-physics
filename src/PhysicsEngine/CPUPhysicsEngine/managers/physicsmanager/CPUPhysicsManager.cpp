#include "CPUPhysicsManager.h"
#include "workers/PhysicsLayerWorker.h"
#include "workers/RigidBodyWorker.h"

CPUPhysicsManager& CPUPhysicsManager::getInstance() {
    static CPUPhysicsManager instance;
    return instance;
}

bool CPUPhysicsManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Initialize physics layer worker
        layerWorker = std::make_shared<PhysicsLayerWorker>();
        if (!layerWorker->initialize()) {
            return false;
        }
        
        // Initialize rigidbody worker
        rigidBodyWorker = std::make_shared<RigidBodyWorker>();
        if (!rigidBodyWorker->initialize()) {
            return false;
        }
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void CPUPhysicsManager::cleanup() {
    if (rigidBodyWorker) {
        rigidBodyWorker->cleanup();
        rigidBodyWorker.reset();
    }
    
    if (layerWorker) {
        layerWorker->cleanup();
        layerWorker.reset();
    }
    
    initialized = false;
}

bool CPUPhysicsManager::isInitialized() const {
    return initialized;
}

void CPUPhysicsManager::updatePhysics(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    // Update rigidbody system
    if (rigidBodyWorker) {
        rigidBodyWorker->updatePhysics(deltaTime);
    }
}

void CPUPhysicsManager::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    // Propagate to rigidbody worker
    if (rigidBodyWorker) {
        rigidBodyWorker->setGravity(x, y, z);
    }
}