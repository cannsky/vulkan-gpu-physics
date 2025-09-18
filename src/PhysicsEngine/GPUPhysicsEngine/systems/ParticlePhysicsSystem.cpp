#include "ParticlePhysicsSystem.h"
#include "../managers/ecsmanager/ECSManager.h"
#include "../managers/vulkanmanager/VulkanManager.h"
#include "../components/ParticleComponent.h"
#include <iostream>

namespace gpu_physics {

ParticlePhysicsSystem::ParticlePhysicsSystem() {
    // Constructor implementation
}

bool ParticlePhysicsSystem::initialize() {
    if (initialized) {
        return true;
    }
    
    if (!ecsManager || !ecsManager->isInitialized()) {
        std::cerr << "ECS Manager not available for ParticlePhysicsSystem" << std::endl;
        return false;
    }
    
    // Note: VulkanManager might not be available in CPU-only builds
    if (vulkanManager && !vulkanManager->isInitialized()) {
        std::cerr << "Warning: Vulkan Manager not initialized, GPU operations will be skipped" << std::endl;
    }
    
    std::cout << "Particle Physics System initialized" << std::endl;
    initialized = true;
    return true;
}

void ParticlePhysicsSystem::cleanup() {
    ecsManager.reset();
    vulkanManager.reset();
    initialized = false;
}

void ParticlePhysicsSystem::updatePhysics(float deltaTime) {
    if (!initialized || !ecsManager) {
        return;
    }
    
    // Upload particle data to GPU
    uploadParticlesToGPU();
    
    // Update uniform buffer with physics parameters
    updateUniformBuffer(deltaTime);
    
    // GPU physics simulation would happen here via compute shaders
    // This is where the actual GPU computation would be dispatched
    
    // Download data back to CPU only if cpu_offloading components exist
    if (shouldPerformCPUOffloading()) {
        downloadParticlesFromGPU();
    }
}

void ParticlePhysicsSystem::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
}

void ParticlePhysicsSystem::uploadParticlesToGPU() {
    if (!vulkanManager || !vulkanManager->isInitialized()) {
        // Skip GPU operations if Vulkan is not available
        return;
    }
    
    uploadRegularParticles();
    uploadCPUOffloadingParticles();
}

void ParticlePhysicsSystem::downloadParticlesFromGPU() {
    if (!vulkanManager || !vulkanManager->isInitialized()) {
        // Skip GPU operations if Vulkan is not available
        return;
    }
    
    // Only download particles that have CPU offloading enabled
    downloadCPUOffloadingParticles();
}

void ParticlePhysicsSystem::updateUniformBuffer(float deltaTime) {
    if (!vulkanManager || !vulkanManager->isInitialized()) {
        // Skip GPU operations if Vulkan is not available
        return;
    }
    
    // Create uniform buffer object for GPU
    struct UniformBufferObject {
        float deltaTime;
        float gravity[3];
        uint32_t particleCount;
    } ubo;
    
    ubo.deltaTime = deltaTime;
    ubo.gravity[0] = gravity.x;
    ubo.gravity[1] = gravity.y;
    ubo.gravity[2] = gravity.z;
    ubo.particleCount = static_cast<uint32_t>(ecsManager->getTotalParticleCount());
    
    // Upload uniform buffer to GPU
    // This would use the BufferManager to update the uniform buffer
}

void ParticlePhysicsSystem::setECSManager(std::shared_ptr<ECSManager> ecsManager) {
    this->ecsManager = ecsManager;
}

void ParticlePhysicsSystem::setVulkanManager(std::shared_ptr<VulkanManager> vulkanManager) {
    this->vulkanManager = vulkanManager;
}

void ParticlePhysicsSystem::uploadRegularParticles() {
    if (!ecsManager) {
        return;
    }
    
    // Get particle component data for upload to GPU
    auto particleData = ecsManager->getParticleComponentData();
    
    if (particleData.empty()) {
        return;
    }
    
    // Upload regular particles to GPU buffer
    // This would use the BufferManager to upload particle data
    std::cout << "Uploading " << particleData.size() << " regular particles to GPU" << std::endl;
}

void ParticlePhysicsSystem::uploadCPUOffloadingParticles() {
    if (!ecsManager) {
        return;
    }
    
    // Get CPU offloading particle component data for upload to GPU
    auto cpuOffloadingData = ecsManager->getParticleComponentWithCPUOffloadingData();
    
    if (cpuOffloadingData.empty()) {
        return;
    }
    
    // Upload CPU offloading particles to GPU buffer
    // This would use the BufferManager to upload particle data
    std::cout << "Uploading " << cpuOffloadingData.size() << " CPU offloading particles to GPU" << std::endl;
}

void ParticlePhysicsSystem::downloadCPUOffloadingParticles() {
    if (!ecsManager) {
        return;
    }
    
    // Only download particles with CPU offloading enabled
    auto cpuOffloadingEntities = ecsManager->getEntitiesWithParticleComponentWithCPUOffloading();
    
    if (cpuOffloadingEntities.empty()) {
        return;
    }
    
    std::cout << "Downloading " << cpuOffloadingEntities.size() << " CPU offloading particles from GPU" << std::endl;
    
    // Download updated particle data from GPU
    // This would use the BufferManager to download particle data
    // Then update the ECS components with the new data
}

bool ParticlePhysicsSystem::shouldPerformCPUOffloading() const {
    if (!ecsManager) {
        return false;
    }
    
    // Check if there are any particles with CPU offloading enabled
    // This uses the static cpu_offloading flag to avoid per-particle checks
    return ecsManager->getParticleComponentWithCPUOffloadingCount() > 0;
}

} // namespace gpu_physics