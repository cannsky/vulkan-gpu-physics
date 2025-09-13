#include "RigidBodySystem.h"
#include "../vulkan/physics/BufferManager.h"
#include "../vulkan/VulkanContext.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

RigidBodySystem::RigidBodySystem(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager)
    : vulkanContext(context), bufferManager(bufferManager), maxRigidBodies(0), nextId(0) {
    // Initialize gravity to Earth's gravity
    ubo.gravity[0] = 0.0f;
    ubo.gravity[1] = -9.81f;
    ubo.gravity[2] = 0.0f;
    ubo.deltaTime = 0.016f;
    ubo.rigidBodyCount = 0;
}

RigidBodySystem::~RigidBodySystem() {
    cleanup();
}

bool RigidBodySystem::initialize(uint32_t maxRigidBodies) {
    this->maxRigidBodies = maxRigidBodies;
    rigidBodies.reserve(maxRigidBodies);
    
    std::cout << "RigidBodySystem initialized with capacity for " << maxRigidBodies << " rigid bodies" << std::endl;
    return true;
}

void RigidBodySystem::cleanup() {
    rigidBodies.clear();
    freeIds.clear();
}

uint32_t RigidBodySystem::createRigidBody(const RigidBody& body) {
    if (rigidBodies.size() >= maxRigidBodies) {
        std::cerr << "Cannot create rigid body: maximum capacity reached" << std::endl;
        return UINT32_MAX;
    }
    
    uint32_t id;
    if (!freeIds.empty()) {
        id = freeIds.back();
        freeIds.pop_back();
        rigidBodies[id] = body;
    } else {
        id = nextId++;
        rigidBodies.push_back(body);
    }
    
    return id;
}

bool RigidBodySystem::removeRigidBody(uint32_t bodyId) {
    if (bodyId >= rigidBodies.size()) {
        return false;
    }
    
    // Mark as available for reuse
    freeIds.push_back(bodyId);
    
    // Zero out the body (inactive)
    memset(&rigidBodies[bodyId], 0, sizeof(RigidBody));
    
    return true;
}

RigidBody* RigidBodySystem::getRigidBody(uint32_t bodyId) {
    if (bodyId >= rigidBodies.size()) {
        return nullptr;
    }
    return &rigidBodies[bodyId];
}

void RigidBodySystem::setGravity(float x, float y, float z) {
    ubo.gravity[0] = x;
    ubo.gravity[1] = y;
    ubo.gravity[2] = z;
}

void RigidBodySystem::updateUniformBuffer(float deltaTime) {
    ubo.deltaTime = deltaTime;
    ubo.rigidBodyCount = static_cast<uint32_t>(rigidBodies.size() - freeIds.size());
}

void RigidBodySystem::uploadRigidBodiesToGPU() {
    // This would upload rigid body data to GPU buffers
    // Implementation depends on BufferManager interface
    std::cout << "Uploading " << rigidBodies.size() << " rigid bodies to GPU" << std::endl;
}

void RigidBodySystem::downloadRigidBodiesFromGPU() {
    // This would download updated rigid body data from GPU
    // Implementation depends on BufferManager interface
    std::cout << "Downloading rigid bodies from GPU" << std::endl;
}

uint32_t RigidBodySystem::createSphere(float x, float y, float z, float radius, float mass) {
    RigidBody body = {};
    
    // Position
    body.position[0] = x;
    body.position[1] = y;
    body.position[2] = z;
    
    // Initialize rotation to identity quaternion
    body.rotation[0] = 1.0f; // w
    body.rotation[1] = 0.0f; // x
    body.rotation[2] = 0.0f; // y
    body.rotation[3] = 0.0f; // z
    
    // Mass properties
    body.mass = mass;
    body.invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    
    // Shape
    body.shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
    body.shapeData[0] = radius;
    
    // Material properties
    body.restitution = 0.6f;
    body.friction = 0.3f;
    
    calculateSphereInertia(body, radius);
    
    return createRigidBody(body);
}

uint32_t RigidBodySystem::createBox(float x, float y, float z, float width, float height, float depth, float mass) {
    RigidBody body = {};
    
    // Position
    body.position[0] = x;
    body.position[1] = y;
    body.position[2] = z;
    
    // Initialize rotation to identity quaternion
    body.rotation[0] = 1.0f; // w
    body.rotation[1] = 0.0f; // x
    body.rotation[2] = 0.0f; // y
    body.rotation[3] = 0.0f; // z
    
    // Mass properties
    body.mass = mass;
    body.invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    
    // Shape
    body.shapeType = static_cast<uint32_t>(RigidBodyShape::BOX);
    body.shapeData[0] = width * 0.5f;  // half-extents
    body.shapeData[1] = height * 0.5f;
    body.shapeData[2] = depth * 0.5f;
    
    // Material properties
    body.restitution = 0.4f;
    body.friction = 0.5f;
    
    calculateBoxInertia(body, width, height, depth);
    
    return createRigidBody(body);
}

uint32_t RigidBodySystem::createStaticPlane(float y) {
    RigidBody body = {};
    
    // Position
    body.position[0] = 0.0f;
    body.position[1] = y;
    body.position[2] = 0.0f;
    
    // Initialize rotation to identity quaternion
    body.rotation[0] = 1.0f; // w
    body.rotation[1] = 0.0f; // x
    body.rotation[2] = 0.0f; // y
    body.rotation[3] = 0.0f; // z
    
    // Static body (infinite mass)
    body.mass = 0.0f;
    body.invMass = 0.0f;
    body.isStatic = 1;
    
    // Material properties
    body.restitution = 0.2f;
    body.friction = 0.8f;
    
    // Zero inertia for static body
    body.inertia[0] = 0.0f;
    body.inertia[1] = 0.0f;
    body.inertia[2] = 0.0f;
    body.invInertia[0] = 0.0f;
    body.invInertia[1] = 0.0f;
    body.invInertia[2] = 0.0f;
    
    return createRigidBody(body);
}

void RigidBodySystem::calculateSphereInertia(RigidBody& body, float radius) {
    if (body.mass <= 0.0f) {
        body.inertia[0] = body.inertia[1] = body.inertia[2] = 0.0f;
        body.invInertia[0] = body.invInertia[1] = body.invInertia[2] = 0.0f;
        return;
    }
    
    float inertia = (2.0f / 5.0f) * body.mass * radius * radius;
    body.inertia[0] = inertia;
    body.inertia[1] = inertia;
    body.inertia[2] = inertia;
    
    body.invInertia[0] = 1.0f / inertia;
    body.invInertia[1] = 1.0f / inertia;
    body.invInertia[2] = 1.0f / inertia;
}

void RigidBodySystem::calculateBoxInertia(RigidBody& body, float width, float height, float depth) {
    if (body.mass <= 0.0f) {
        body.inertia[0] = body.inertia[1] = body.inertia[2] = 0.0f;
        body.invInertia[0] = body.invInertia[1] = body.invInertia[2] = 0.0f;
        return;
    }
    
    float mass12 = body.mass / 12.0f;
    body.inertia[0] = mass12 * (height * height + depth * depth);
    body.inertia[1] = mass12 * (width * width + depth * depth);
    body.inertia[2] = mass12 * (width * width + height * height);
    
    body.invInertia[0] = 1.0f / body.inertia[0];
    body.invInertia[1] = 1.0f / body.inertia[1];
    body.invInertia[2] = 1.0f / body.inertia[2];
}