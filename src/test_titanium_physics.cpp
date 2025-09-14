#include "physics_engine.h"
#include "cpu_physics/CPUPhysicsSystem.h"
#include "managers/logmanager/Logger.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    // Configure logging
    Logger::getInstance().setLogLevel(LogLevel::INFO);
    Logger::getInstance().enableCategory(LogCategory::PHYSICS);
    Logger::getInstance().enableCategory(LogCategory::RIGIDBODY);
    Logger::getInstance().enableConsoleOutput(true);
    
    std::cout << "Titanium Physics Engine - Hybrid Architecture Test" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    // Create physics engine
    PhysicsEngine physicsEngine;
    
    // Initialize without Vulkan (CPU only)
    if (!physicsEngine.initialize(0, 10)) { // 0 particles, 10 rigidbodies max
        std::cerr << "Failed to initialize physics engine!" << std::endl;
        return -1;
    }
    
    std::cout << "Physics engine initialized successfully" << std::endl;
    
    // Create physics layers
    uint32_t defaultLayer = physicsEngine.createPhysicsLayer("Default");
    uint32_t staticLayer = physicsEngine.createPhysicsLayer("Static");
    
    std::cout << "Created physics layers: Default=" << defaultLayer << ", Static=" << staticLayer << std::endl;
    
    // Set layer interactions
    physicsEngine.setLayerInteraction(defaultLayer, staticLayer, true);
    physicsEngine.setLayerInteraction(defaultLayer, defaultLayer, true);
    
    // Create some test rigidbodies
    uint32_t groundId = physicsEngine.createRigidBody(0.0f, -1.0f, 0.0f, 10.0f, 0.2f, 10.0f, 0.0f, staticLayer); // Static ground
    uint32_t box1Id = physicsEngine.createRigidBody(0.0f, 5.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, defaultLayer);
    uint32_t box2Id = physicsEngine.createRigidBody(1.5f, 8.0f, 0.0f, 1.0f, 1.0f, 1.0f, 2.0f, defaultLayer);
    
    std::cout << "Created rigidbodies: Ground=" << groundId << ", Box1=" << box1Id << ", Box2=" << box2Id << std::endl;
    
    // Set gravity
    physicsEngine.setGravity(0.0f, -9.81f, 0.0f);
    std::cout << "Set gravity to (0, -9.81, 0)" << std::endl;
    
    // Run simulation for a few seconds
    std::cout << "\nRunning simulation..." << std::endl;
    
    const float deltaTime = 1.0f / 60.0f; // 60 FPS
    const int totalFrames = 300; // 5 seconds
    
    for (int frame = 0; frame < totalFrames; ++frame) {
        physicsEngine.updatePhysics(deltaTime);
        
        // Print status every 60 frames (1 second)
        if (frame % 60 == 0) {
            auto* box1 = physicsEngine.getRigidBody(box1Id);
            auto* box2 = physicsEngine.getRigidBody(box2Id);
            
            if (box1 && box2) {
                std::cout << "Frame " << frame << ": ";
                std::cout << "Box1 Y=" << box1->transform.position[1] << ", ";
                std::cout << "Box2 Y=" << box2->transform.position[1] << std::endl;
            }
        }
        
        // Small delay to make it more realistic
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Final status
    auto* box1 = physicsEngine.getRigidBody(box1Id);
    auto* box2 = physicsEngine.getRigidBody(box2Id);
    
    if (box1 && box2) {
        std::cout << "\nFinal positions:" << std::endl;
        std::cout << "Box1: (" << box1->transform.position[0] << ", " << box1->transform.position[1] << ", " << box1->transform.position[2] << ")" << std::endl;
        std::cout << "Box2: (" << box2->transform.position[0] << ", " << box2->transform.position[1] << ", " << box2->transform.position[2] << ")" << std::endl;
    }
    
    // Cleanup
    physicsEngine.cleanup();
    
    std::cout << "\nTitanium Physics Engine test completed successfully!" << std::endl;
    return 0;
}