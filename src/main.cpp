#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <random>
#include "managers/vulkanmanager/VulkanManager.h"
#include "managers/physicsmanager/PhysicsManager.h"
#include "managers/particlemanager/ParticleManager.h"
#include "factories/RigidBodyFactory.h"
#include "managers/physicsmanager/workers/PhysicsLayerWorker.h"
#include "managers/logmanager/Logger.h"

int main() {
    // Configure logging system
    Logger::getInstance().setLogLevel(LogLevel::INFO);
    Logger::getInstance().enableCategory(LogCategory::PHYSICS);
    Logger::getInstance().enableCategory(LogCategory::PARTICLES);
    Logger::getInstance().enableCategory(LogCategory::PERFORMANCE);
    Logger::getInstance().setOutputFile("physics_simulation.log");
    
    std::cout << "Vulkan GPU Physics - Unified physics simulation system" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    LOG_INFO(LogCategory::GENERAL, "Starting Vulkan GPU Physics simulation");
    
    // Initialize managers in order
    auto& vulkanManager = VulkanManager::getInstance();
    if (!vulkanManager.initialize()) {
        std::cerr << "Failed to initialize Vulkan manager!" << std::endl;
        return -1;
    }
    
    auto& physicsManager = PhysicsManager::getInstance();
    if (!physicsManager.initialize()) {
        std::cerr << "Failed to initialize physics manager!" << std::endl;
        vulkanManager.cleanup();
        return -1;
    }
    
    auto& particleManager = ParticleManager::getInstance();
    
    // Set up layer system using PhysicsLayerWorker through PhysicsManager
    auto physicsLayerWorker = physicsManager.getLayerWorker();
    auto particleLayer = physicsLayerWorker->createLayer("Particles");
    auto staticLayer = physicsLayerWorker->createLayer("Static");
    
    // Set up random number generation for particle initialization
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);
    std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);
    std::uniform_real_distribution<float> massDist(0.5f, 2.0f);
    
    // Add some random particles
    const int numParticles = 100;
    std::cout << "\nAdding " << numParticles << " particles to the simulation..." << std::endl;
    
    for (int i = 0; i < numParticles; ++i) {
        Particle particle = {};
        particle.position[0] = posDist(gen);
        particle.position[1] = posDist(gen) + 10.0f; // Start above ground
        particle.position[2] = posDist(gen);
        particle.velocity[0] = velDist(gen);
        particle.velocity[1] = velDist(gen);
        particle.velocity[2] = velDist(gen);
        particle.mass = massDist(gen);
        
        if (!particleManager.addParticle(particle)) {
            std::cerr << "Failed to add particle " << i << std::endl;
            break;
        }
    }
    
    // Create some rigid bodies using the factory
    auto& rigidBodyFactory = RigidBodyFactory::getInstance();
    
    // Create a ground plane
    auto groundPlane = rigidBodyFactory.createStaticPlane(0.0f, staticLayer);
    std::cout << "Created ground plane at y=0" << std::endl;
    
    // Create some spheres
    auto sphere1 = rigidBodyFactory.createSphere(0.0f, 5.0f, 0.0f, 1.0f, 1.0f, particleLayer);
    auto sphere2 = rigidBodyFactory.createSphere(2.0f, 8.0f, 0.0f, 0.5f, 0.5f, particleLayer);
    std::cout << "Created rigid body spheres" << std::endl;
    
    // Set gravity
    physicsManager.setGravity(0.0f, -9.81f, 0.0f);
    
    std::cout << "Starting physics simulation..." << std::endl;
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
        physicsManager.updatePhysics(deltaTime);
        
        totalTime += deltaTime;
        frameCount++;
        
        // Log frame time for performance monitoring
        Logger::getInstance().logFrameTime(deltaTime);
        
        // Print statistics every second
        if (totalTime >= 1.0f) {
            auto particles = particleManager.getParticles();
            
            // Calculate average height of particles
            float avgHeight = 0.0f;
            float minHeight = particles.empty() ? 0.0f : particles[0].position[1];
            float maxHeight = particles.empty() ? 0.0f : particles[0].position[1];
            
            for (const auto& particle : particles) {
                avgHeight += particle.position[1];
                minHeight = std::min(minHeight, particle.position[1]);
                maxHeight = std::max(maxHeight, particle.position[1]);
            }
            if (!particles.empty()) {
                avgHeight /= particles.size();
            }
            
            // Log performance statistics
            Logger::getInstance().logParticleCount(static_cast<uint32_t>(particles.size()));
            LOG_PERFORMANCE_INFO("FPS: " + std::to_string(frameCount) + 
                               ", Avg Height: " + std::to_string(avgHeight));
            
            std::cout << "FPS: " << frameCount 
                      << ", Particles: " << particles.size()
                      << ", Layers: " << physicsLayerWorker->getLayerCount()
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
    
    // Cleanup managers
    physicsManager.cleanup();
    vulkanManager.cleanup();
    
    std::cout << "Simulation ended." << std::endl;
    return 0;
}