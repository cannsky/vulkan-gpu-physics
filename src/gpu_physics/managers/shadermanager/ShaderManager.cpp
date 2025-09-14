#include "ShaderManager.h"
#include "../vulkanmanager/VulkanManager.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

ShaderManager& ShaderManager::getInstance() {
    static ShaderManager instance;
    return instance;
}

bool ShaderManager::initialize() {
    if (initialized) {
        return true;
    }
    
    // Ensure VulkanManager is initialized
    auto& vulkanManager = VulkanManager::getInstance();
    if (!vulkanManager.isInitialized()) {
        return false;
    }
    
    initialized = true;
    return true;
}

void ShaderManager::cleanup() {
    clearCache();
    initialized = false;
}

bool ShaderManager::isInitialized() const {
    return initialized;
}

VkShaderModule ShaderManager::createShaderModule(const std::vector<char>& code) {
    auto& vulkanManager = VulkanManager::getInstance();
    VkDevice device = vulkanManager.getLogicalDevice();
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }
    
    return shaderModule;
}

VkShaderModule ShaderManager::loadShaderFromFile(const std::string& filename) {
    auto code = readFile(filename);
    return createShaderModule(code);
}

VkShaderModule ShaderManager::loadShaderFromSPV(const std::string& spvFilename) {
    std::string fullPath = getShaderPath(spvFilename);
    return loadShaderFromFile(fullPath);
}

VkShaderModule ShaderManager::getOrCreateShader(const std::string& key, const std::string& filename) {
    auto it = shaderCache.find(key);
    if (it != shaderCache.end()) {
        return it->second;
    }
    
    VkShaderModule shader = loadShaderFromSPV(filename);
    cacheShader(key, shader);
    return shader;
}

void ShaderManager::cacheShader(const std::string& key, VkShaderModule shader) {
    shaderCache[key] = shader;
}

bool ShaderManager::hasShader(const std::string& key) const {
    return shaderCache.find(key) != shaderCache.end();
}

VkShaderModule ShaderManager::getParticleComputeShader() {
    return getOrCreateShader("particle_compute", "particle_physics.comp.spv");
}

VkShaderModule ShaderManager::getRigidBodyComputeShader() {
    return getOrCreateShader("rigidbody_compute", "rigidbody_physics.comp.spv");
}

VkShaderModule ShaderManager::getCollisionComputeShader() {
    return getOrCreateShader("collision_compute", "collision_detection.comp.spv");
}

void ShaderManager::destroyShader(VkShaderModule shader) {
    if (shader == VK_NULL_HANDLE) {
        return;
    }
    
    auto& vulkanManager = VulkanManager::getInstance();
    if (vulkanManager.isInitialized()) {
        vkDestroyShaderModule(vulkanManager.getLogicalDevice(), shader, nullptr);
    }
}

void ShaderManager::clearCache() {
    for (auto& pair : shaderCache) {
        destroyShader(pair.second);
    }
    shaderCache.clear();
}

std::vector<char> ShaderManager::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

std::string ShaderManager::getShaderPath(const std::string& filename) {
    // Default shader directory - adjust based on your project structure
    return "shaders/" + filename;
}