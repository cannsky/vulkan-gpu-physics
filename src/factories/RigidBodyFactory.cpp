#include "RigidBodyFactory.h"
#include <cmath>
#include <cstring>

RigidBodyFactory& RigidBodyFactory::getInstance() {
    static RigidBodyFactory instance;
    return instance;
}

std::unique_ptr<RigidBody> RigidBodyFactory::createSphere(float x, float y, float z, float radius, 
                                                        float mass, PhysicsLayerWorker::LayerId layer) {
    auto body = std::make_unique<RigidBody>();
    std::memset(body.get(), 0, sizeof(RigidBody));
    
    // Set basic properties
    body->position[0] = x;
    body->position[1] = y;
    body->position[2] = z;
    body->mass = mass;
    body->invMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
    body->layer = layer;
    
    // Set sphere shape
    body->shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
    body->shapeData[0] = radius; // radius in first element
    
    // Initialize rotation quaternion (identity)
    body->rotation[0] = 1.0f; // w
    body->rotation[1] = 0.0f; // x
    body->rotation[2] = 0.0f; // y
    body->rotation[3] = 0.0f; // z
    
    // Calculate inertia for sphere
    calculateSphereInertia(*body, radius);
    
    // Apply default material
    applyDefaultMaterial(*body);
    
    return body;
}

std::unique_ptr<RigidBody> RigidBodyFactory::createBox(float x, float y, float z, float width, float height, float depth, 
                                                     float mass, PhysicsLayerWorker::LayerId layer) {
    auto body = std::make_unique<RigidBody>();
    std::memset(body.get(), 0, sizeof(RigidBody));
    
    // Set basic properties
    body->position[0] = x;
    body->position[1] = y;
    body->position[2] = z;
    body->mass = mass;
    body->invMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
    body->layer = layer;
    
    // Set box shape
    body->shapeType = static_cast<uint32_t>(RigidBodyShape::BOX);
    body->shapeData[0] = width * 0.5f;  // half-width
    body->shapeData[1] = height * 0.5f; // half-height
    body->shapeData[2] = depth * 0.5f;  // half-depth
    
    // Initialize rotation quaternion (identity)
    body->rotation[0] = 1.0f; // w
    body->rotation[1] = 0.0f; // x
    body->rotation[2] = 0.0f; // y
    body->rotation[3] = 0.0f; // z
    
    // Calculate inertia for box
    calculateBoxInertia(*body, width, height, depth);
    
    // Apply default material
    applyDefaultMaterial(*body);
    
    return body;
}

std::unique_ptr<RigidBody> RigidBodyFactory::createStaticPlane(float y, PhysicsLayerWorker::LayerId layer) {
    auto body = std::make_unique<RigidBody>();
    std::memset(body.get(), 0, sizeof(RigidBody));
    
    // Set basic properties
    body->position[0] = 0.0f;
    body->position[1] = y;
    body->position[2] = 0.0f;
    body->mass = 0.0f; // Static objects have zero mass
    body->invMass = 0.0f;
    body->layer = layer;
    body->isStatic = 1;
    
    // Set plane shape
    body->shapeType = static_cast<uint32_t>(RigidBodyShape::PLANE);
    body->shapeData[0] = 0.0f; // normal.x
    body->shapeData[1] = 1.0f; // normal.y (up-facing)
    body->shapeData[2] = 0.0f; // normal.z
    body->shapeData[3] = y;    // distance from origin
    
    // Initialize rotation quaternion (identity)
    body->rotation[0] = 1.0f; // w
    body->rotation[1] = 0.0f; // x
    body->rotation[2] = 0.0f; // y
    body->rotation[3] = 0.0f; // z
    
    // Static objects don't need inertia
    body->inertia[0] = 0.0f;
    body->inertia[1] = 0.0f;
    body->inertia[2] = 0.0f;
    body->invInertia[0] = 0.0f;
    body->invInertia[1] = 0.0f;
    body->invInertia[2] = 0.0f;
    
    // Apply default material
    applyDefaultMaterial(*body);
    
    return body;
}

std::unique_ptr<RigidBody> RigidBodyFactory::createCustomRigidBody(const RigidBody& template_body, PhysicsLayerWorker::LayerId layer) {
    auto body = std::make_unique<RigidBody>(template_body);
    body->layer = layer;
    return body;
}

void RigidBodyFactory::setDefaultMaterial(float restitution, float friction) {
    defaultRestitution = restitution;
    defaultFriction = friction;
}

void RigidBodyFactory::getDefaultMaterial(float& restitution, float& friction) const {
    restitution = defaultRestitution;
    friction = defaultFriction;
}

void RigidBodyFactory::calculateSphereInertia(RigidBody& body, float radius) {
    if (body.mass <= 0.0f) {
        body.inertia[0] = 0.0f;
        body.inertia[1] = 0.0f;
        body.inertia[2] = 0.0f;
        body.invInertia[0] = 0.0f;
        body.invInertia[1] = 0.0f;
        body.invInertia[2] = 0.0f;
        return;
    }
    
    float I = (2.0f / 5.0f) * body.mass * radius * radius;
    body.inertia[0] = I;
    body.inertia[1] = I;
    body.inertia[2] = I;
    body.invInertia[0] = 1.0f / I;
    body.invInertia[1] = 1.0f / I;
    body.invInertia[2] = 1.0f / I;
}

void RigidBodyFactory::calculateBoxInertia(RigidBody& body, float width, float height, float depth) {
    if (body.mass <= 0.0f) {
        body.inertia[0] = 0.0f;
        body.inertia[1] = 0.0f;
        body.inertia[2] = 0.0f;
        body.invInertia[0] = 0.0f;
        body.invInertia[1] = 0.0f;
        body.invInertia[2] = 0.0f;
        return;
    }
    
    float Ix = (body.mass / 12.0f) * (height * height + depth * depth);
    float Iy = (body.mass / 12.0f) * (width * width + depth * depth);
    float Iz = (body.mass / 12.0f) * (width * width + height * height);
    
    body.inertia[0] = Ix;
    body.inertia[1] = Iy;
    body.inertia[2] = Iz;
    body.invInertia[0] = 1.0f / Ix;
    body.invInertia[1] = 1.0f / Iy;
    body.invInertia[2] = 1.0f / Iz;
}

void RigidBodyFactory::applyDefaultMaterial(RigidBody& body) {
    body.restitution = defaultRestitution;
    body.friction = defaultFriction;
}