#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>

class VulkanContext;
class BufferManager;

class ComputePipeline {
public:
    ComputePipeline(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager);
    ~ComputePipeline();
    
    bool initialize();
    void cleanup();
    
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    VkDescriptorPool getDescriptorPool() const { return descriptorPool; }
    VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkPipeline getPipeline() const { return computePipeline; }

private:
    bool createDescriptorSetLayout();
    bool createComputePipeline();
    bool createDescriptorPool();
    bool createDescriptorSets();
    bool loadShader(const std::string& filename, std::vector<char>& shaderCode);
    
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<BufferManager> bufferManager;
    
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline computePipeline = VK_NULL_HANDLE;
};