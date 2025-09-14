#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <random>
#include <iomanip>
#include "physics_engine.h"
#include "cpu_physics/CPUPhysicsSystem.h"
#include "managers/logmanager/Logger.h"

int main() {
    // Configure logging system
    Logger::getInstance().setLogLevel(LogLevel::INFO);
    Logger::getInstance().enableCategory(LogCategory::PHYSICS);
    Logger::getInstance().enableCategory(LogCategory::RIGIDBODY);
    Logger::getInstance().enableCategory(LogCategory::PERFORMANCE);
    Logger::getInstance().enableConsoleOutput(true);
    Logger::getInstance().setOutputFile("titanium_physics_simulation.log");
    
    std::cout << "Titanium Physics Engine - CPU-Only Demo" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    LOG_INFO(LogCategory::GENERAL, "Starting Titanium Physics CPU-only simulation");
    
    // Initialize Titanium Physics Engine (CPU-only)
    PhysicsEngine physicsEngine;
    
    uint32_t maxParticles = 0;        // No GPU particles in CPU-only mode
    uint32_t maxRigidBodies = 50;
    
    if (!physicsEngine.initialize(maxParticles, maxRigidBodies)) {
        std::cerr << "Failed to initialize Titanium Physics Engine!" << std::endl;
        return -1;
    }
    
    std::cout << "Titanium Physics Engine initialized successfully" << std::endl;
    std::cout << "GPU Physics: Disabled (CPU-only mode)" << std::endl;
    std::cout << "CPU Physics: Enabled" << std::endl;
    
    // Create physics layers for collision filtering
    uint32_t dynamicLayer = physicsEngine.createPhysicsLayer("Dynamic");
    uint32_t staticLayer = physicsEngine.createPhysicsLayer("Static");
    
    // Set up layer interactions
    physicsEngine.setLayerInteraction(dynamicLayer, staticLayer, true);    // Dynamic can hit static
    physicsEngine.setLayerInteraction(dynamicLayer, dynamicLayer, true);   // Dynamic can hit dynamic
    
    std::cout << "\nCreated physics layers:" << std::endl;
    std::cout << "- Dynamic: " << dynamicLayer << std::endl;
    std::cout << "- Static: " << staticLayer << std::endl;
    
    // Create static environment (ground and walls)
    auto groundId = physicsEngine.createRigidBody(0.0f, -1.0f, 0.0f, 20.0f, 0.4f, 20.0f, 0.0f, staticLayer);
    auto leftWallId = physicsEngine.createRigidBody(-10.0f, 5.0f, 0.0f, 0.4f, 10.0f, 20.0f, 0.0f, staticLayer);
    auto rightWallId = physicsEngine.createRigidBody(10.0f, 5.0f, 0.0f, 0.4f, 10.0f, 20.0f, 0.0f, staticLayer);
    
    std::cout << "\nCreated static environment:" << std::endl;
    std::cout << "- Ground: " << groundId << std::endl;
    std::cout << "- Left wall: " << leftWallId << std::endl;
    std::cout << "- Right wall: " << rightWallId << std::endl;
    
    // Create dynamic rigidbodies
    std::vector<uint32_t> dynamicBodies;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);
    std::uniform_real_distribution<float> sizeDist(0.5f, 1.5f);
    std::uniform_real_distribution<float> massDist(0.5f, 3.0f);
    
    const int numDynamicBodies = 15;
    for (int i = 0; i < numDynamicBodies; ++i) {
        float x = posDist(gen);
        float y = 5.0f + i * 1.5f; // Stack them vertically
        float z = posDist(gen);
        float size = sizeDist(gen);
        float mass = massDist(gen);
        
        auto bodyId = physicsEngine.createRigidBody(x, y, z, size, size, size, mass, dynamicLayer);
        dynamicBodies.push_back(bodyId);
    }
    
    std::cout << "\nCreated " << numDynamicBodies << " dynamic rigidbodies" << std::endl;
    
    // Set gravity
    physicsEngine.setGravity(0.0f, -9.81f, 0.0f);
    
    std::cout << "\nStarting physics simulation..." << std::endl;
    std::cout << "Running for 10 seconds..." << std::endl;
    std::cout << "\nSimulation Statistics:" << std::endl;
    
    // Simulation loop
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastTime = startTime;
    float totalTime = 0.0f;
    int frameCount = 0;
    const float targetFrameTime = 1.0f / 60.0f; // 60 FPS
    const float simulationDuration = 10.0f; // Run for 10 seconds
    
    while (totalTime < simulationDuration) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        float totalElapsed = std::chrono::duration<float>(currentTime - startTime).count();
        lastTime = currentTime;
        
        // Clamp delta time to prevent large jumps
        deltaTime = std::min(deltaTime, 0.016f); // Max 16ms
        
        // Update physics
        physicsEngine.updatePhysics(deltaTime);
        
        totalTime += deltaTime;
        frameCount++;
        
        // Print statistics every second
        if (frameCount % 60 == 0) {
            // Calculate average height of dynamic bodies
            float avgHeight = 0.0f;
            float minHeight = 100.0f;
            float maxHeight = -100.0f;
            int activeBodies = 0;
            
            for (auto bodyId : dynamicBodies) {
                auto* body = physicsEngine.getRigidBody(bodyId);
                if (body) {
                    float height = body->transform.position[1];
                    avgHeight += height;
                    minHeight = std::min(minHeight, height);
                    maxHeight = std::max(maxHeight, height);
                    activeBodies++;
                }
            }
            
            if (activeBodies > 0) {
                avgHeight /= activeBodies;
            }
            
            // Log performance statistics
            LOG_PERFORMANCE_INFO("Time: " + std::to_string(totalElapsed) + 
                               "s, Avg RigidBody Height: " + std::to_string(avgHeight));
            
            std::cout << "Time: " << std::fixed << std::setprecision(1) << totalElapsed << "s"
                      << ", RigidBodies: " << activeBodies
                      << ", Avg Height: " << std::setprecision(2) << avgHeight
                      << ", Min: " << minHeight
                      << ", Max: " << maxHeight
                      << std::endl;
        }
        
        // Maintain target frame rate
        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto frameTime = std::chrono::duration<float>(frameEnd - currentTime).count();
        
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(targetFrameTime - frameTime)
            );
        }
    }
    
    // Final statistics
    std::cout << "\nFinal positions of dynamic bodies:" << std::endl;
    for (size_t i = 0; i < dynamicBodies.size() && i < 5; ++i) {
        auto* body = physicsEngine.getRigidBody(dynamicBodies[i]);
        if (body) {
            std::cout << "Body " << dynamicBodies[i] << ": ("
                      << std::fixed << std::setprecision(2)
                      << body->transform.position[0] << ", "
                      << body->transform.position[1] << ", "
                      << body->transform.position[2] << ")" << std::endl;
        }
    }
    
    std::cout << "\nAverage FPS: " << (frameCount / totalTime) << std::endl;
    
    // Cleanup
    physicsEngine.cleanup();
    
    std::cout << "\nTitanium Physics CPU-only simulation completed successfully!" << std::endl;
    return 0;
}