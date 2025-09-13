#include "ParticleManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include "../../physics/components/BufferManager.h"
#include "../../physics/VulkanContext.h"
#include <iostream>
#include <cstring>

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
        
        // Initialize particle manager with default max particles
        return initializeInternal(1024);
        
    } catch (const std::exception& e) {
        cleanup();
        return false;
    }
}

bool ParticleManager::initializeInternal(uint32_t maxParticles) {
    this->maxParticles = maxParticles;
    particles.reserve(maxParticles);
    
    // Initialize gravity to Earth's gravity
    ubo.gravity[0] = 0.0f;
    ubo.gravity[1] = -9.81f;
    ubo.gravity[2] = 0.0f;
    
    std::cout << "Particle manager initialized with " << maxParticles << " max particles" << std::endl;
    initialized = true;
    return true;
}

void ParticleManager::cleanup() {
    particles.clear();
    initialized = false;
}

bool ParticleManager::isInitialized() const {
    return initialized;
}

bool ParticleManager::addParticle(const Particle& particle) {
    if (!initialized) {
        return false;
    }
    
    if (particles.size() >= maxParticles) {
        std::cerr << "Cannot add particle: maximum capacity reached!" << std::endl;
        return false;
    }
    
    particles.push_back(particle);
    return true;
}

std::vector<Particle> ParticleManager::getParticles() const {
    return particles;
}

uint32_t ParticleManager::getParticleCount() const {
    return static_cast<uint32_t>(particles.size());
}

uint32_t ParticleManager::getMaxParticles() const {
    return maxParticles;
}

void ParticleManager::updatePhysics(float deltaTime) {
    if (!initialized) {
        return;
    }
    
    updateUniformBuffer(deltaTime);
    uploadParticlesToGPU();
    // GPU compute dispatch would happen here in a full implementation
    downloadParticlesFromGPU();
}

void ParticleManager::setGravity(float x, float y, float z) {
    ubo.gravity[0] = x;
    ubo.gravity[1] = y;
    ubo.gravity[2] = z;
}

void ParticleManager::updateUniformBuffer(float deltaTime) {
    if (!initialized || !bufferManager) {
        return;
    }
    
    ubo.deltaTime = deltaTime;
    ubo.particleCount = static_cast<uint32_t>(particles.size());
    
    // For now, just log the update - actual GPU operations would need vulkan context
    std::cout << "Updating uniform buffer: deltaTime=" << deltaTime 
              << ", particleCount=" << ubo.particleCount << std::endl;
}

void ParticleManager::uploadParticlesToGPU() {
    if (!initialized || particles.empty()) {
        return;
    }
    
    // For now, just log the operation - actual GPU operations would need vulkan context
    std::cout << "Uploading " << particles.size() << " particles to GPU" << std::endl;
}

void ParticleManager::downloadParticlesFromGPU() {
    if (!initialized || particles.empty()) {
        return;
    }
    
    // For now, just log the operation - actual GPU operations would need vulkan context
    std::cout << "Downloading " << particles.size() << " particles from GPU" << std::endl;
}

void ParticleManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // Placeholder implementation - would need command buffer and queue operations
    std::cout << "Copying buffer of size " << size << std::endl;
}

uint32_t ParticleManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // Placeholder implementation - would need physical device memory properties
    return 0;
}