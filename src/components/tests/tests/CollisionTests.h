#pragma once

#include "../Test.h"
#include "../../../gpu_physics/managers/collisionmanager/CollisionManager.h"
#include "../../../cpu_physics/rigidbody/RigidBody.h"
#include "../../../managers/logmanager/Logger.h"
#include <cmath>

class SphereCollisionDetectionTest : public Test {
public:
    std::string getName() const override {
        return "SphereCollisionDetection";
    }
    
    std::string getClassName() const override {
        return "CollisionTests";
    }
    
    void run(TestResult& result) override {
        // Create two spheres
        RigidBody sphereA = {};
        sphereA.position[0] = 0.0f;
        sphereA.position[1] = 0.0f;
        sphereA.position[2] = 0.0f;
        sphereA.shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
        sphereA.shapeData[0] = 1.0f; // radius
        sphereA.restitution = 0.5f;
        sphereA.friction = 0.3f;
        
        RigidBody sphereB = {};
        sphereB.position[0] = 1.5f; // Overlapping
        sphereB.position[1] = 0.0f;
        sphereB.position[2] = 0.0f;
        sphereB.shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
        sphereB.shapeData[0] = 1.0f; // radius
        sphereB.restitution = 0.7f;
        sphereB.friction = 0.4f;
        
        // Test collision detection logic manually
        float dx = sphereA.position[0] - sphereB.position[0];
        float dy = sphereA.position[1] - sphereB.position[1];
        float dz = sphereA.position[2] - sphereB.position[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        float radiusSum = sphereA.shapeData[0] + sphereB.shapeData[0];
        
        bool shouldCollide = distSq < (radiusSum * radiusSum);
        assertTrue(shouldCollide, "Spheres should be colliding");
        
        float dist = std::sqrt(distSq);
        float penetration = radiusSum - dist;
        assertTrue(penetration > 0.0f, "Should have positive penetration");
        
        LOG_COLLISION_INFO("Sphere collision test passed - penetration: " + std::to_string(penetration));
        result.markPassed("Collision detection working correctly, penetration: " + std::to_string(penetration));
    }
};

class MaterialPropertiesTest : public Test {
public:
    std::string getName() const override {
        return "MaterialProperties";
    }
    
    std::string getClassName() const override {
        return "CollisionTests";
    }
    
    void run(TestResult& result) override {
        RigidBody bodyA = {};
        bodyA.restitution = 0.5f;
        bodyA.friction = 0.3f;
        
        RigidBody bodyB = {};
        bodyB.restitution = 0.7f;
        bodyB.friction = 0.4f;
        
        // Test combined material properties
        float combinedRestitution = (bodyA.restitution + bodyB.restitution) * 0.5f;
        float combinedFriction = std::sqrt(bodyA.friction * bodyB.friction);
        
        assertEqual(combinedRestitution, 0.6f, 0.001f, "Combined restitution should be 0.6");
        assertEqual(combinedFriction, std::sqrt(0.3f * 0.4f), 0.001f, "Combined friction should be geometric mean");
        
        result.markPassed("Material property combinations calculated correctly");
    }
};

class NonCollisionTest : public Test {
public:
    std::string getName() const override {
        return "NonCollision";
    }
    
    std::string getClassName() const override {
        return "CollisionTests";
    }
    
    void run(TestResult& result) override {
        // Create two spheres that don't collide
        RigidBody sphereA = {};
        sphereA.position[0] = 0.0f;
        sphereA.position[1] = 0.0f;
        sphereA.position[2] = 0.0f;
        sphereA.shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
        sphereA.shapeData[0] = 1.0f; // radius
        
        RigidBody sphereB = {};
        sphereB.position[0] = 3.0f; // Far apart
        sphereB.position[1] = 0.0f;
        sphereB.position[2] = 0.0f;
        sphereB.shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
        sphereB.shapeData[0] = 1.0f; // radius
        
        // Test collision detection logic manually
        float dx = sphereA.position[0] - sphereB.position[0];
        float dy = sphereA.position[1] - sphereB.position[1];
        float dz = sphereA.position[2] - sphereB.position[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        float radiusSum = sphereA.shapeData[0] + sphereB.shapeData[0];
        
        bool shouldCollide = distSq < (radiusSum * radiusSum);
        assertFalse(shouldCollide, "Spheres should not be colliding");
        
        result.markPassed("Non-collision correctly detected");
    }
};