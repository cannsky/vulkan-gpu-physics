#include "physics_engine.h"
#include "cpu_physics/CPUPhysicsSystem.h"
#include "managers/logmanager/Logger.h"

// Only include GPU physics if Vulkan is available
#ifdef VULKAN_AVAILABLE
#include "gpu_physics/GPUPhysicsSystem.h"
#include "managers/vulkanmanager/VulkanManager.h"
#endif

PhysicsEngine::PhysicsEngine() {
    LOG_INFO(LogCategory::PHYSICS, "Initializing Titanium Physics Engine");
}

PhysicsEngine::~PhysicsEngine() {
    cleanup();
}

bool PhysicsEngine::initialize(uint32_t maxParticles, uint32_t maxRigidBodies) {
    if (initialized) {
        LOG_INFO(LogCategory::PHYSICS, "Physics engine already initialized");
        return true;
    }
    
    LOG_INFO(LogCategory::PHYSICS, "Initializing hybrid physics system - GPU: " + 
             std::to_string(maxParticles) + " particles, CPU: " + 
             std::to_string(maxRigidBodies) + " rigidbodies");
    
#ifdef VULKAN_AVAILABLE
    // Initialize GPU Physics System for particles/fluids
    auto& vulkanManager = VulkanManager::getInstance();
    if (vulkanManager.isInitialized()) {
        gpuPhysics = std::make_unique<gpu_physics::GPUPhysicsSystem>(vulkanManager.getContext());
        if (!gpuPhysics->initialize(maxParticles)) {
            LOG_ERROR(LogCategory::PHYSICS, "Failed to initialize GPU physics system");
            return false;
        }
        LOG_INFO(LogCategory::PHYSICS, "GPU physics system initialized for particles/fluids");
    } else {
        LOG_WARN(LogCategory::PHYSICS, "Vulkan not available, GPU physics disabled");
    }
#else
    LOG_INFO(LogCategory::PHYSICS, "Vulkan not compiled in, GPU physics disabled");
#endif
    
    // Initialize CPU Physics System for rigidbodies (ECS-based)
    cpuPhysics = std::make_unique<cpu_physics::CPUPhysicsSystem>();
    if (!cpuPhysics->initialize(maxRigidBodies)) {
        LOG_ERROR(LogCategory::PHYSICS, "Failed to initialize CPU physics system");
        return false;
    }
    LOG_INFO(LogCategory::PHYSICS, "CPU physics system initialized for rigidbodies");
    
    // Set initial gravity for both systems
    setGravity(gravity.x, gravity.y, gravity.z);
    
    initialized = true;
    LOG_INFO(LogCategory::PHYSICS, "Titanium Physics Engine successfully initialized");
    return true;
}

void PhysicsEngine::cleanup() {
    if (!initialized) {
        return;
    }
    
    LOG_INFO(LogCategory::PHYSICS, "Cleaning up Titanium Physics Engine");
    
    if (cpuPhysics) {
        cpuPhysics->cleanup();
        cpuPhysics.reset();
    }
    
#ifdef VULKAN_AVAILABLE
    if (gpuPhysics) {
        gpuPhysics->cleanup();
        gpuPhysics.reset();
    }
#endif
    
    initialized = false;
    LOG_INFO(LogCategory::PHYSICS, "Titanium Physics Engine cleanup complete");
}

void PhysicsEngine::updatePhysics(float deltaTime) {
    if (!initialized) {
        return;
    }
    
#ifdef VULKAN_AVAILABLE
    // Update GPU physics (particles/fluids) if available
    if (gpuPhysics) {
        gpuPhysics->updatePhysics(deltaTime);
    }
#endif
    
    // Update CPU physics (rigidbodies)
    if (cpuPhysics) {
        cpuPhysics->updatePhysics(deltaTime);
    }
}

void PhysicsEngine::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
#ifdef VULKAN_AVAILABLE
    if (gpuPhysics) {
        gpuPhysics->setGravity(x, y, z);
    }
#endif
    
    if (cpuPhysics) {
        cpuPhysics->setGravity(x, y, z);
    }
    
    LOG_INFO(LogCategory::PHYSICS, "Gravity set to (" + std::to_string(x) + ", " + 
             std::to_string(y) + ", " + std::to_string(z) + ")");
}

// GPU Physics (Particles/Fluids) Interface
bool PhysicsEngine::addParticle(float x, float y, float z, float vx, float vy, float vz, float mass) {
#ifdef VULKAN_AVAILABLE
    if (!gpuPhysics) {
        LOG_WARN(LogCategory::PHYSICS, "GPU physics not available, cannot add particle");
        return false;
    }
    
    return gpuPhysics->addParticle(x, y, z, vx, vy, vz, mass);
#else
    LOG_WARN(LogCategory::PHYSICS, "GPU physics not compiled in, cannot add particle");
    return false;
#endif
}

size_t PhysicsEngine::getParticleCount() const {
#ifdef VULKAN_AVAILABLE
    if (!gpuPhysics) {
        return 0;
    }
    
    return gpuPhysics->getParticleCount();
#else
    return 0;
#endif
}

// CPU Physics (RigidBodies) Interface
uint32_t PhysicsEngine::createRigidBody(float x, float y, float z, float width, float height, float depth, float mass, uint32_t layer) {
    if (!cpuPhysics) {
        LOG_ERROR(LogCategory::PHYSICS, "CPU physics not available, cannot create rigidbody");
        return 0;
    }
    
    uint32_t bodyId = cpuPhysics->createRigidBody(x, y, z, width, height, depth, mass, layer);
    
    if (bodyId != 0) {
        LOG_INFO(LogCategory::RIGIDBODY, "Created rigidbody " + std::to_string(bodyId) + 
                 " at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + 
                 ") with dimensions (" + std::to_string(width) + ", " + std::to_string(height) + ", " + std::to_string(depth) + ")");
    }
    
    return bodyId;
}

bool PhysicsEngine::removeRigidBody(uint32_t bodyId) {
    if (!cpuPhysics) {
        return false;
    }
    
    bool success = cpuPhysics->removeRigidBody(bodyId);
    if (success) {
        LOG_INFO(LogCategory::RIGIDBODY, "Removed rigidbody " + std::to_string(bodyId));
    }
    
    return success;
}

cpu_physics::RigidBodyComponent* PhysicsEngine::getRigidBody(uint32_t bodyId) {
    if (!cpuPhysics) {
        return nullptr;
    }
    
    return cpuPhysics->getRigidBody(bodyId);
}

// Layer system for collision filtering
uint32_t PhysicsEngine::createPhysicsLayer(const std::string& name) {
    if (!cpuPhysics) {
        LOG_ERROR(LogCategory::PHYSICS, "CPU physics not available, cannot create layer");
        return 0;
    }
    
    uint32_t layerId = cpuPhysics->createLayer(name);
    LOG_INFO(LogCategory::PHYSICS, "Created physics layer '" + name + "' with ID " + std::to_string(layerId));
    return layerId;
}

bool PhysicsEngine::setLayerInteraction(uint32_t layer1, uint32_t layer2, bool canInteract) {
    if (!cpuPhysics) {
        return false;
    }
    
    bool success = cpuPhysics->setLayerInteraction(layer1, layer2, canInteract);
    if (success) {
        LOG_INFO(LogCategory::PHYSICS, "Set layer interaction between " + std::to_string(layer1) + 
                 " and " + std::to_string(layer2) + ": " + (canInteract ? "enabled" : "disabled"));
    }
    
    return success;
}

bool PhysicsEngine::canLayersInteract(uint32_t layer1, uint32_t layer2) const {
    if (!cpuPhysics) {
        return false;
    }
    
    return cpuPhysics->canLayersInteract(layer1, layer2);
}