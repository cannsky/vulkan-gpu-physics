#include "VulkanPhysics.h"
#include <iostream>
#include <fstream>
#include <cstring>

VulkanPhysics::VulkanPhysics(std::shared_ptr<VulkanContext> context) 
    : vulkanContext(context), maxParticles(0) {
    // Initialize gravity to Earth's gravity
    ubo.gravity[0] = 0.0f;
    ubo.gravity[1] = -9.81f;
    ubo.gravity[2] = 0.0f;
}

VulkanPhysics::~VulkanPhysics() {
    cleanup();
}

bool VulkanPhysics::initialize(uint32_t maxParticles) {
    this->maxParticles = maxParticles;
    particles.reserve(maxParticles);
    
    if (!createBuffers(maxParticles)) {
        std::cerr << "Failed to create buffers!" << std::endl;
        return false;
    }
    
    if (!createComputePipeline()) {
        std::cerr << "Failed to create compute pipeline!" << std::endl;
        return false;
    }
    
    if (!createDescriptorSets()) {
        std::cerr << "Failed to create descriptor sets!" << std::endl;
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
    
    std::cout << "Physics system initialized with " << maxParticles << " max particles" << std::endl;
    return true;
}

void VulkanPhysics::cleanup() {
    auto device = vulkanContext->getDevice();
    
    if (computePipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, computePipeline, nullptr);
        computePipeline = VK_NULL_HANDLE;
    }
    
    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
    
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
    
    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }
    
    if (particleBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, particleBuffer, nullptr);
        particleBuffer = VK_NULL_HANDLE;
    }
    
    if (particleBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, particleBufferMemory, nullptr);
        particleBufferMemory = VK_NULL_HANDLE;
    }
    
    if (uniformBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, uniformBuffer, nullptr);
        uniformBuffer = VK_NULL_HANDLE;
    }
    
    if (uniformBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, uniformBufferMemory, nullptr);
        uniformBufferMemory = VK_NULL_HANDLE;
    }
}

bool VulkanPhysics::addParticle(const Particle& particle) {
    if (particles.size() >= maxParticles) {
        std::cerr << "Cannot add more particles, max limit reached!" << std::endl;
        return false;
    }
    
    particles.push_back(particle);
    return true;
}

void VulkanPhysics::updatePhysics(float deltaTime) {
    if (particles.empty()) return;
    
    updateUniformBuffer(deltaTime);
    
    // Copy particle data to GPU buffer
    void* data;
    vkMapMemory(vulkanContext->getDevice(), particleBufferMemory, 0, 
                particles.size() * sizeof(Particle), 0, &data);
    memcpy(data, particles.data(), particles.size() * sizeof(Particle));
    vkUnmapMemory(vulkanContext->getDevice(), particleBufferMemory);
    
    recordComputeCommandBuffer();
    
    // Submit compute work
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCommandBuffer;
    
    vkQueueSubmit(vulkanContext->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanContext->getComputeQueue());
    
    // Read back results
    vkMapMemory(vulkanContext->getDevice(), particleBufferMemory, 0, 
                particles.size() * sizeof(Particle), 0, &data);
    memcpy(particles.data(), data, particles.size() * sizeof(Particle));
    vkUnmapMemory(vulkanContext->getDevice(), particleBufferMemory);
}

std::vector<Particle> VulkanPhysics::getParticles() const {
    return particles;
}

void VulkanPhysics::setGravity(float x, float y, float z) {
    ubo.gravity[0] = x;
    ubo.gravity[1] = y;
    ubo.gravity[2] = z;
}

bool VulkanPhysics::createBuffers(uint32_t maxParticles) {
    VkDeviceSize bufferSize = sizeof(Particle) * maxParticles;
    
    // Create particle buffer
    if (!createBuffer(bufferSize, 
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     particleBuffer, particleBufferMemory)) {
        return false;
    }
    
    // Create uniform buffer
    VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);
    if (!createBuffer(uniformBufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniformBuffer, uniformBufferMemory)) {
        return false;
    }
    
    return true;
}

bool VulkanPhysics::createComputePipeline() {
    // Load compute shader
    std::vector<char> computeShaderCode;
    if (!loadShader("shaders/particle_physics.comp.spv", computeShaderCode)) {
        std::cerr << "Failed to load compute shader!" << std::endl;
        return false;
    }
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = computeShaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(computeShaderCode.data());
    
    VkShaderModule computeShaderModule;
    if (vkCreateShaderModule(vulkanContext->getDevice(), &createInfo, nullptr, 
                            &computeShaderModule) != VK_SUCCESS) {
        std::cerr << "Failed to create compute shader module!" << std::endl;
        return false;
    }
    
    // Create descriptor set layout
    VkDescriptorSetLayoutBinding particleLayoutBinding{};
    particleLayoutBinding.binding = 0;
    particleLayoutBinding.descriptorCount = 1;
    particleLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    particleLayoutBinding.pImmutableSamplers = nullptr;
    particleLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    VkDescriptorSetLayoutBinding uniformLayoutBinding{};
    uniformLayoutBinding.binding = 1;
    uniformLayoutBinding.descriptorCount = 1;
    uniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformLayoutBinding.pImmutableSamplers = nullptr;
    uniformLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        particleLayoutBinding, uniformLayoutBinding
    };
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(vulkanContext->getDevice(), &layoutInfo, nullptr, 
                                   &descriptorSetLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create descriptor set layout!" << std::endl;
        vkDestroyShaderModule(vulkanContext->getDevice(), computeShaderModule, nullptr);
        return false;
    }
    
    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    
    if (vkCreatePipelineLayout(vulkanContext->getDevice(), &pipelineLayoutInfo, nullptr, 
                              &pipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout!" << std::endl;
        vkDestroyShaderModule(vulkanContext->getDevice(), computeShaderModule, nullptr);
        return false;
    }
    
    // Create compute pipeline
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";
    
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;
    
    if (vkCreateComputePipelines(vulkanContext->getDevice(), VK_NULL_HANDLE, 1, 
                                &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create compute pipeline!" << std::endl;
        vkDestroyShaderModule(vulkanContext->getDevice(), computeShaderModule, nullptr);
        return false;
    }
    
    vkDestroyShaderModule(vulkanContext->getDevice(), computeShaderModule, nullptr);
    return true;
}

bool VulkanPhysics::createDescriptorSets() {
    // Create descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes{};
    poolSizes.resize(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    
    if (vkCreateDescriptorPool(vulkanContext->getDevice(), &poolInfo, nullptr, 
                              &descriptorPool) != VK_SUCCESS) {
        return false;
    }
    
    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    
    if (vkAllocateDescriptorSets(vulkanContext->getDevice(), &allocInfo, 
                                &descriptorSet) != VK_SUCCESS) {
        return false;
    }
    
    // Update descriptor set
    VkDescriptorBufferInfo particleBufferInfo{};
    particleBufferInfo.buffer = particleBuffer;
    particleBufferInfo.offset = 0;
    particleBufferInfo.range = maxParticles * sizeof(Particle);
    
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = uniformBuffer;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(UniformBufferObject);
    
    std::vector<VkWriteDescriptorSet> descriptorWrites{};
    descriptorWrites.resize(2);
    
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &particleBufferInfo;
    
    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &uniformBufferInfo;
    
    vkUpdateDescriptorSets(vulkanContext->getDevice(), 
                          static_cast<uint32_t>(descriptorWrites.size()), 
                          descriptorWrites.data(), 0, nullptr);
    
    return true;
}

bool VulkanPhysics::loadShader(const std::string& filename, std::vector<char>& shaderCode) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << std::endl;
        return false;
    }
    
    size_t fileSize = (size_t) file.tellg();
    shaderCode.resize(fileSize);
    
    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();
    
    return true;
}

void VulkanPhysics::updateUniformBuffer(float deltaTime) {
    ubo.deltaTime = deltaTime;
    ubo.particleCount = static_cast<uint32_t>(particles.size());
    
    void* data;
    vkMapMemory(vulkanContext->getDevice(), uniformBufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(vulkanContext->getDevice(), uniformBufferMemory);
}

void VulkanPhysics::recordComputeCommandBuffer() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if (vkBeginCommandBuffer(computeCommandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    
    vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                           pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    
    uint32_t groupCount = (particles.size() + 31) / 32; // 32 threads per group
    vkCmdDispatch(computeCommandBuffer, groupCount, 1, 1);
    
    if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

uint32_t VulkanPhysics::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type!");
}

bool VulkanPhysics::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties, VkBuffer& buffer,
                                VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(vulkanContext->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        return false;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanContext->getDevice(), buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        return false;
    }
    
    vkBindBufferMemory(vulkanContext->getDevice(), buffer, bufferMemory, 0);
    return true;
}

void VulkanPhysics::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanContext->getCommandPool();
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanContext->getDevice(), &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(vulkanContext->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanContext->getComputeQueue());
    
    vkFreeCommandBuffers(vulkanContext->getDevice(), vulkanContext->getCommandPool(), 1, &commandBuffer);
}