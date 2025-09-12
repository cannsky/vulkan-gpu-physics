#pragma once

#include "RigidBody.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class VulkanContext;
class BufferManager;

class RigidBodySystem {
public:
    RigidBodySystem(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager);
    ~RigidBodySystem();
    
    bool initialize(uint32_t maxRigidBodies = 512);
    void cleanup();
    
    uint32_t createRigidBody(const RigidBody& body);
    bool removeRigidBody(uint32_t bodyId);
    RigidBody* getRigidBody(uint32_t bodyId);
    
    void setGravity(float x, float y, float z);
    void updateUniformBuffer(float deltaTime);
    void uploadRigidBodiesToGPU();
    void downloadRigidBodiesFromGPU();
    
    uint32_t getRigidBodyCount() const { return static_cast<uint32_t>(rigidBodies.size()); }
    uint32_t getMaxRigidBodies() const { return maxRigidBodies; }
    
    // Create common shapes
    uint32_t createSphere(float x, float y, float z, float radius, float mass = 1.0f);
    uint32_t createBox(float x, float y, float z, float width, float height, float depth, float mass = 1.0f);
    uint32_t createStaticPlane(float y = 0.0f); // Ground plane

private:
    void calculateSphereInertia(RigidBody& body, float radius);
    void calculateBoxInertia(RigidBody& body, float width, float height, float depth);
    
    std::shared_ptr<VulkanContext> vulkanContext;
    std::shared_ptr<BufferManager> bufferManager;
    
    std::vector<RigidBody> rigidBodies;
    std::vector<uint32_t> freeIds; // For ID reuse
    uint32_t maxRigidBodies;
    uint32_t nextId;
    
    struct UniformBufferObject {
        float deltaTime;
        float gravity[3];
        uint32_t rigidBodyCount;
        float padding[3]; // For alignment
    } ubo;
};