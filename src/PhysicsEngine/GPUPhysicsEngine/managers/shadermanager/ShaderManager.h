#pragma once

#include "../../../managers/BaseManager.h"
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Manager for shader compilation and management.
 * Provides singleton access to shader resources.
 */
class ShaderManager : public BaseManager {
public:
    static ShaderManager& getInstance();
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Shader operations
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkShaderModule loadShaderFromFile(const std::string& filename);
    VkShaderModule loadShaderFromSPV(const std::string& spvFilename);
    
    // Shader cache management
    VkShaderModule getOrCreateShader(const std::string& key, const std::string& filename);
    void cacheShader(const std::string& key, VkShaderModule shader);
    bool hasShader(const std::string& key) const;
    
    // Common shaders
    VkShaderModule getParticleComputeShader();
    VkShaderModule getRigidBodyComputeShader();
    VkShaderModule getCollisionComputeShader();
    
    // Utility
    void destroyShader(VkShaderModule shader);
    void clearCache();

private:
    ShaderManager() = default;
    ~ShaderManager() = default;
    
    bool initialized = false;
    std::unordered_map<std::string, VkShaderModule> shaderCache;
    
    // Helper methods
    std::vector<char> readFile(const std::string& filename);
    std::string getShaderPath(const std::string& filename);
};