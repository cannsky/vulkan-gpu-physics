#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <random>
#include "physics_engine/physics_engine.h"
#include "physics_engine/cpu_physics_engine/CPUPhysicsEngine.h"
#ifdef VULKAN_AVAILABLE
#include "physics_engine/gpu_physics_engine/managers/vulkanmanager/VulkanManager.h"
#endif
#include "physics_engine/managers/logmanager/Logger.h"

int main(int argc, char* argv[]) {
    // Parse command line arguments
    bool cpuOnlyMode = false;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--cpu-only" || arg == "-c") {
            cpuOnlyMode = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Titanium Physics Engine - Hybrid GPU/CPU Physics System" << std::endl;
            std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --cpu-only, -c    Run in CPU-only mode (disable Vulkan/GPU physics)" << std::endl;
            std::cout << "  --help, -h        Show this help message" << std::endl;
            return 0;
        }
    }
    // Configure logging system
    Logger::getInstance().setLogLevel(LogLevel::INFO);
    Logger::getInstance().enableCategory(LogCategory::PHYSICS);
    Logger::getInstance().enableCategory(LogCategory::RIGIDBODY);
    Logger::getInstance().enableCategory(LogCategory::PARTICLES);
    Logger::getInstance().enableCategory(LogCategory::PERFORMANCE);
    Logger::getInstance().enableConsoleOutput(true);
    Logger::getInstance().setOutputFile("titanium_physics_simulation.log");
    
    std::cout << "Titanium Physics Engine - Hybrid GPU/CPU Physics System" << std::endl;
    std::cout << "=======================================================" << std::endl;
    
    if (cpuOnlyMode) {
        std::cout << "Running in CPU-only mode (GPU physics disabled)" << std::endl;
    }
    
    LOG_INFO(LogCategory::GENERAL, "Starting Titanium Physics simulation");
    
    // Try to initialize Vulkan for GPU physics (optional, unless forced CPU-only)
    bool vulkanAvailable = false;
#ifdef VULKAN_AVAILABLE
    if (!cpuOnlyMode) {
        auto& vulkanManager = VulkanManager::getInstance();
        if (vulkanManager.initialize()) {
            vulkanAvailable = true;
            std::cout << "Vulkan initialized successfully - GPU physics available" << std::endl;
        } else {
            std::cout << "Vulkan not available - using CPU-only physics" << std::endl;
        }
    } else {
        std::cout << "GPU physics disabled by user request - using CPU-only physics" << std::endl;
    }
#else
    std::cout << "Vulkan not compiled - using CPU-only physics" << std::endl;
#endif
    
    // Initialize Titanium Physics Engine
    PhysicsEngine physicsEngine;
    
    uint32_t maxParticles = vulkanAvailable ? 1000 : 0; // Only use particles if Vulkan is available
    uint32_t maxRigidBodies = 50;
    
    if (!physicsEngine.initialize(maxParticles, maxRigidBodies)) {
        std::cerr << "Failed to initialize Titanium Physics Engine!" << std::endl;
#ifdef VULKAN_AVAILABLE
        if (vulkanAvailable) {
            vulkanManager.cleanup();
        }
#endif
        return -1;
    }
    
    std::cout << "Titanium Physics Engine initialized successfully" << std::endl;
    std::cout << "GPU Physics: " << (vulkanAvailable ? "Enabled" : "Disabled") << std::endl;
    std::cout << "CPU Physics: Enabled" << std::endl;
    
    // Create physics layers for collision filtering
    uint32_t dynamicLayer = physicsEngine.createPhysicsLayer("Dynamic");
    uint32_t staticLayer = physicsEngine.createPhysicsLayer("Static");
    uint32_t particleLayer = physicsEngine.createPhysicsLayer("Particles");
    
    // Set up layer interactions
    physicsEngine.setLayerInteraction(dynamicLayer, staticLayer, true);    // Dynamic can hit static
    physicsEngine.setLayerInteraction(dynamicLayer, dynamicLayer, true);   // Dynamic can hit dynamic
    physicsEngine.setLayerInteraction(particleLayer, staticLayer, true);   // Particles can hit static
    physicsEngine.setLayerInteraction(particleLayer, dynamicLayer, true);  // Particles can hit dynamic
    
    std::cout << "\nCreated physics layers:" << std::endl;
    std::cout << "- Dynamic: " << dynamicLayer << std::endl;
    std::cout << "- Static: " << staticLayer << std::endl;
    std::cout << "- Particles: " << particleLayer << std::endl;
    
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
    
    const int numDynamicBodies = 10;
    for (int i = 0; i < numDynamicBodies; ++i) {
        float x = posDist(gen);
        float y = 5.0f + i * 2.0f; // Stack them vertically
        float z = posDist(gen);
        float size = sizeDist(gen);
        float mass = massDist(gen);
        
        auto bodyId = physicsEngine.createRigidBody(x, y, z, size, size, size, mass, dynamicLayer);
        dynamicBodies.push_back(bodyId);
    }
    
    std::cout << "\nCreated " << numDynamicBodies << " dynamic rigidbodies" << std::endl;
    
    // Add particles if GPU physics is available
    if (vulkanAvailable) {
        const int numParticles = 200;
        std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);
        
        for (int i = 0; i < numParticles; ++i) {
            float x = posDist(gen);
            float y = 15.0f + posDist(gen) * 5.0f; // Start high above the rigidbodies
            float z = posDist(gen);
            float vx = velDist(gen);
            float vy = velDist(gen);
            float vz = velDist(gen);
            
            physicsEngine.addParticle(x, y, z, vx, vy, vz, 0.1f);
        }
        
        std::cout << "Added " << numParticles << " particles" << std::endl;
    }
    
    // Set gravity
    physicsEngine.setGravity(0.0f, -9.81f, 0.0f);
    
    std::cout << "\nStarting physics simulation..." << std::endl;
    std::cout << "Press Ctrl+C to stop the simulation" << std::endl;
    std::cout << "\nSimulation Statistics:" << std::endl;
    
    // Simulation loop
    auto lastTime = std::chrono::high_resolution_clock::now();
    float totalTime = 0.0f;
    int frameCount = 0;
    const float targetFrameTime = 1.0f / 60.0f; // 60 FPS
    
    while (true) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Clamp delta time to prevent large jumps
        deltaTime = std::min(deltaTime, 0.016f); // Max 16ms
        
        // Update physics
        physicsEngine.updatePhysics(deltaTime);
        
        totalTime += deltaTime;
        frameCount++;
        
        // Print statistics every second
        if (totalTime >= 1.0f) {
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
            LOG_PERFORMANCE_INFO("FPS: " + std::to_string(frameCount) + 
                               ", Avg RigidBody Height: " + std::to_string(avgHeight));
            
            std::cout << "FPS: " << frameCount 
                      << ", RigidBodies: " << activeBodies
                      << ", Particles: " << physicsEngine.getParticleCount()
                      << ", Avg Height: " << avgHeight
                      << ", Min Height: " << minHeight
                      << ", Max Height: " << maxHeight
                      << std::endl;
            
            totalTime = 0.0f;
            frameCount = 0;
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
    
    // Cleanup
    physicsEngine.cleanup();
#ifdef VULKAN_AVAILABLE
    if (vulkanAvailable) {
        vulkanManager.cleanup();
    }
#endif
    
    std::cout << "Titanium Physics simulation ended." << std::endl;
    return 0;
}