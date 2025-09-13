#include "PhysicsEngine.h"
#include "../../vulkan/VulkanContext.h"
#include "../../vulkan/physics/BufferManager.h"
#include "../../vulkan/physics/ComputePipeline.h"
#include "../../managers/particlemanager/ParticleManager.h"
#include "../../managers/logmanager/Logger.h"
#include <iostream>

PhysicsEngine::PhysicsEngine(std::shared_ptr<VulkanContext> context)
    : vulkanContext(context), maxParticles(0) {
}

PhysicsEngine::~PhysicsEngine() {
    cleanup();
}

bool PhysicsEngine::initialize(uint32_t maxParticles) {
    this->maxParticles = maxParticles;
    
    LOG_PHYSICS_INFO("Initializing PhysicsEngine with " + std::to_string(maxParticles) + " max particles");
    
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
    
    // Initialize particle manager
    auto& particleManager = ParticleManager::getInstance();
    if (!particleManager.initialize()) {
        std::cerr << "Failed to initialize particle manager!" << std::endl;
        return false;
    }
    
    // Allocate command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanContext->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(vulkanContext->getDevice(), &allocInfo, &computeCommandBuffer) != VK_SUCCESS) {
        std::cerr << "Failed to allocate command buffer!" << std::endl;
        return false;
    }
    
    std::cout << "Physics engine initialized with " << maxParticles << " max particles" << std::endl;
    return true;
}

void PhysicsEngine::cleanup() {
    if (computeCommandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(vulkanContext->getDevice(), vulkanContext->getCommandPool(), 1, &computeCommandBuffer);
        computeCommandBuffer = VK_NULL_HANDLE;
    }
    
    computePipeline.reset();
    bufferManager.reset();
}

bool PhysicsEngine::addParticle(const Particle& particle) {
    return ParticleManager::getInstance().addParticle(particle);
}

void PhysicsEngine::updatePhysics(float deltaTime) {
    auto& particleManager = ParticleManager::getInstance();
    if (particleManager.getParticleCount() == 0) {
        return;
    }
    
    // Upload particle data to GPU
    particleManager.uploadParticlesToGPU();
    
    // Update uniform buffer with current frame data
    particleManager.updateUniformBuffer(deltaTime);
    
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
    particleManager.downloadParticlesFromGPU();
}

std::vector<Particle> PhysicsEngine::getParticles() const {
    return ParticleManager::getInstance().getParticles();
}

void PhysicsEngine::setGravity(float x, float y, float z) {
    ParticleManager::getInstance().setGravity(x, y, z);
}

void PhysicsEngine::recordComputeCommandBuffer() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    
    vkBeginCommandBuffer(computeCommandBuffer, &beginInfo);
    
    vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->getPipeline());
    
    VkDescriptorSet descriptorSet = computePipeline->getDescriptorSet();
    vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
                           computePipeline->getPipelineLayout(), 0, 1, 
                           &descriptorSet, 0, nullptr);
    
    uint32_t particleCount = ParticleManager::getInstance().getParticleCount();
    uint32_t groupCount = (particleCount + 31) / 32; // Round up to nearest multiple of 32
    vkCmdDispatch(computeCommandBuffer, groupCount, 1, 1);
    
    vkEndCommandBuffer(computeCommandBuffer);
}