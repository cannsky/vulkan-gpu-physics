#include "GPUPhysicsEngine.h"
#include "../components/vulkan/VulkanContext.h"
#include "../components/vulkan/physics/BufferManager.h"
#include "../components/vulkan/physics/ComputePipeline.h"
#include "../managers/particlemanager/ParticleManager.h"
#include "../managers/logmanager/Logger.h"
#include <iostream>

namespace gpu_physics {

GPUPhysicsEngine::GPUPhysicsEngine(std::shared_ptr<VulkanContext> context)
    : vulkanContext(context), maxParticles(0) {
    LOG_INFO(LogCategory::PHYSICS, "Creating GPU Physics System");
}

GPUPhysicsEngine::~GPUPhysicsEngine() {
    cleanup();
}

bool GPUPhysicsEngine::initialize(uint32_t maxParticles) {
    this->maxParticles = maxParticles;
    particles.reserve(maxParticles);
    
    LOG_INFO(LogCategory::PHYSICS, "Initializing GPU Physics System with " + 
             std::to_string(maxParticles) + " max particles");
    
    if (!vulkanContext) {
        LOG_ERROR(LogCategory::PHYSICS, "No Vulkan context available for GPU physics");
        return false;
    }
    
    // Initialize buffer manager
    bufferManager = std::make_shared<BufferManager>(vulkanContext);
    if (!bufferManager->initialize(maxParticles)) {
        LOG_ERROR(LogCategory::PHYSICS, "Failed to initialize buffer manager");
        return false;
    }
    
    // Initialize compute pipeline
    computePipeline = std::make_shared<ComputePipeline>(vulkanContext, bufferManager);
    if (!computePipeline->initialize()) {
        LOG_ERROR(LogCategory::PHYSICS, "Failed to initialize compute pipeline");
        return false;
    }
    
    // Allocate command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanContext->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(vulkanContext->getDevice(), &allocInfo, &computeCommandBuffer) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::PHYSICS, "Failed to allocate command buffer for GPU physics");
        return false;
    }
    
    LOG_INFO(LogCategory::PHYSICS, "GPU Physics System initialized successfully");
    return true;
}

void GPUPhysicsEngine::cleanup() {
    if (computeCommandBuffer != VK_NULL_HANDLE && vulkanContext) {
        vkFreeCommandBuffers(vulkanContext->getDevice(), vulkanContext->getCommandPool(), 1, &computeCommandBuffer);
        computeCommandBuffer = VK_NULL_HANDLE;
    }
    
    computePipeline.reset();
    bufferManager.reset();
    particles.clear();
    
    LOG_INFO(LogCategory::PHYSICS, "GPU Physics System cleanup complete");
}

bool GPUPhysicsEngine::addParticle(const Particle& particle) {
    if (particles.size() >= maxParticles) {
        LOG_WARN(LogCategory::PARTICLES, "Cannot add particle: maximum capacity reached");
        return false;
    }
    
    particles.push_back(particle);
    return true;
}

bool GPUPhysicsEngine::addParticle(float x, float y, float z, float vx, float vy, float vz, float mass) {
    Particle particle = {};
    particle.position[0] = x;
    particle.position[1] = y;
    particle.position[2] = z;
    particle.velocity[0] = vx;
    particle.velocity[1] = vy;
    particle.velocity[2] = vz;
    particle.mass = mass;
    
    return addParticle(particle);
}

std::vector<Particle> GPUPhysicsEngine::getParticles() const {
    return particles;
}

size_t GPUPhysicsEngine::getParticleCount() const {
    return particles.size();
}

void GPUPhysicsEngine::updatePhysics(float deltaTime) {
    if (particles.empty() || !vulkanContext || !computePipeline) {
        return;
    }
    
    // Upload particle data to GPU
    uploadParticlesToGPU();
    
    // Record compute command buffer
    recordComputeCommandBuffer();
    
    // Submit compute work
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCommandBuffer;
    
    vkQueueSubmit(vulkanContext->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanContext->getComputeQueue());
    
    // Download updated particle data from GPU
    downloadParticlesFromGPU();
}

void GPUPhysicsEngine::setGravity(float x, float y, float z) {
    gravity.x = x;
    gravity.y = y;
    gravity.z = z;
    
    LOG_INFO(LogCategory::PHYSICS, "GPU Physics gravity set to (" + 
             std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

void GPUPhysicsEngine::uploadParticlesToGPU() {
    if (!bufferManager || particles.empty()) {
        return;
    }
    
    // This would upload particle data to GPU buffers
    // Implementation depends on BufferManager interface
    LOG_PHYSICS_INFO("Uploading " + std::to_string(particles.size()) + " particles to GPU");
}

void GPUPhysicsEngine::downloadParticlesFromGPU() {
    if (!bufferManager || particles.empty()) {
        return;
    }
    
    // This would download updated particle data from GPU
    // Implementation depends on BufferManager interface
    LOG_PHYSICS_INFO("Downloading " + std::to_string(particles.size()) + " particles from GPU");
}

void GPUPhysicsEngine::recordComputeCommandBuffer() {
    if (!computePipeline || computeCommandBuffer == VK_NULL_HANDLE) {
        return;
    }
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    
    vkBeginCommandBuffer(computeCommandBuffer, &beginInfo);
    
    vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->getPipeline());
    
    VkDescriptorSet descriptorSet = computePipeline->getDescriptorSet();
    vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
                           computePipeline->getPipelineLayout(), 0, 1, 
                           &descriptorSet, 0, nullptr);
    
    uint32_t particleCount = static_cast<uint32_t>(particles.size());
    uint32_t groupCount = (particleCount + 31) / 32; // Round up to nearest multiple of 32
    vkCmdDispatch(computeCommandBuffer, groupCount, 1, 1);
    
    vkEndCommandBuffer(computeCommandBuffer);
}

} // namespace gpu_physics