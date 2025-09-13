#include "ParticleManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../physicsmanager/PhysicsManager.h"
#include "../../particles/ParticleSystem.h"

ParticleManager& ParticleManager::getInstance() {
    static ParticleManager instance;
    return instance;
}

bool ParticleManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Ensure VulkanManager is initialized
        auto& vulkanManager = VulkanManager::getInstance();
        if (!vulkanManager.isInitialized()) {
            return false;
        }
        
        // Get buffer manager from PhysicsManager
        auto& physicsManager = PhysicsManager::getInstance();
        auto bufferManager = physicsManager.getBufferManager();
        if (!bufferManager) {
            return false;
        }
        
        // Create particle system
        particleSystem = std::make_shared<ParticleSystem>(vulkanManager.getContext(), bufferManager);
        if (!particleSystem->initialize(physicsManager.getMaxParticles())) {
            cleanup();
            return false;
        }
        
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

void ParticleManager::cleanup() {
    if (particleSystem) {
        particleSystem->cleanup();
        particleSystem.reset();
    }
    
    initialized = false;
}

bool ParticleManager::isInitialized() const {
    return initialized;
}

bool ParticleManager::addParticle(const Particle& particle) {
    if (!particleSystem) {
        return false;
    }
    return particleSystem->addParticle(particle);
}

std::vector<Particle> ParticleManager::getParticles() const {
    if (!particleSystem) {
        return {};
    }
    return particleSystem->getParticles();
}

uint32_t ParticleManager::getParticleCount() const {
    if (!particleSystem) {
        return 0;
    }
    return particleSystem->getParticleCount();
}

uint32_t ParticleManager::getMaxParticles() const {
    if (!particleSystem) {
        return 0;
    }
    return particleSystem->getMaxParticles();
}

void ParticleManager::updatePhysics(float deltaTime) {
    if (!particleSystem) {
        return;
    }
    
    particleSystem->updateUniformBuffer(deltaTime);
    uploadParticlesToGPU();
    // GPU compute dispatch would happen here in a full implementation
    downloadParticlesFromGPU();
}

void ParticleManager::setGravity(float x, float y, float z) {
    if (!particleSystem) {
        return;
    }
    particleSystem->setGravity(x, y, z);
}

void ParticleManager::uploadParticlesToGPU() {
    if (!particleSystem) {
        return;
    }
    particleSystem->uploadParticlesToGPU();
}

void ParticleManager::downloadParticlesFromGPU() {
    if (!particleSystem) {
        return;
    }
    particleSystem->downloadParticlesFromGPU();
}