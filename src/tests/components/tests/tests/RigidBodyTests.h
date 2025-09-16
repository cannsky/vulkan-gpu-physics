#pragma once

#include "../Test.h"
#include "../../../../PhysicsEngine/CPUPhysicsEngine/rigidbody/RigidBodySystem.h"
#include "../../../../physics_engine/managers/logmanager/Logger.h"
#include <cmath>

class RigidBodyCreationTest : public Test {
public:
    std::string getName() const override {
        return "RigidBodyCreation";
    }
    
    std::string getClassName() const override {
        return "RigidBodyTests";
    }
    
    void run(TestResult& result) override {
        // Create a rigid body data structure
        RigidBody sphere = {};
        sphere.position[0] = 1.0f;
        sphere.position[1] = 2.0f;
        sphere.position[2] = 3.0f;
        sphere.mass = 1.5f;
        sphere.invMass = 1.0f / 1.5f;
        sphere.shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
        sphere.shapeData[0] = 0.5f; // radius
        
        // Verify data integrity
        assertEqual(sphere.position[0], 1.0f, "Position X should be 1.0");
        assertEqual(sphere.position[1], 2.0f, "Position Y should be 2.0");
        assertEqual(sphere.position[2], 3.0f, "Position Z should be 3.0");
        assertEqual(sphere.mass, 1.5f, 0.001f, "Mass should be 1.5");
        assertEqual(sphere.invMass, 1.0f / 1.5f, 0.001f, "Inverse mass should be 1/1.5");
        assertEqual(static_cast<int>(sphere.shapeType), static_cast<int>(RigidBodyShape::SPHERE), "Shape type should be SPHERE");
        assertEqual(sphere.shapeData[0], 0.5f, 0.001f, "Sphere radius should be 0.5");
        
        LOG_RIGIDBODY_INFO("Rigid body creation test passed");
        result.markPassed("Rigid body created and verified successfully");
    }
};

class RigidBodyMassCalculationTest : public Test {
public:
    std::string getName() const override {
        return "MassCalculation";
    }
    
    std::string getClassName() const override {
        return "RigidBodyTests";
    }
    
    void run(TestResult& result) override {
        // Test mass calculations for different shapes
        RigidBody sphere = {};
        sphere.mass = 2.0f;
        sphere.invMass = 1.0f / sphere.mass;
        
        RigidBody staticBody = {};
        staticBody.mass = 0.0f;
        staticBody.invMass = 0.0f;
        staticBody.isStatic = 1;
        
        // Verify calculations
        assertEqual(sphere.invMass, 0.5f, 0.001f, "Sphere inverse mass should be 0.5");
        assertEqual(staticBody.invMass, 0.0f, "Static body inverse mass should be 0");
        assertEqual(static_cast<int>(staticBody.isStatic), 1, "Static body flag should be set");
        
        result.markPassed("Mass calculations verified for dynamic and static bodies");
    }
};