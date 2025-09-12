#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <random>
#include "physics/VulkanContext.h"
#include "physics/VulkanPhysics.h"

int main() {
    std::cout << "Tulpar Physics - GPU-based physics simulation using Vulkan" << std::endl;
    std::cout << "============================================================" << std::endl;
    
    // Initialize Vulkan context
    auto vulkanContext = std::make_shared<VulkanContext>();
    if (!vulkanContext->initialize()) {
        std::cerr << "Failed to initialize Vulkan context!" << std::endl;
        return -1;
    }
    
    // Initialize physics system
    auto physicsSystem = std::make_unique<VulkanPhysics>(vulkanContext);
    if (!physicsSystem->initialize(1024)) {
        std::cerr << "Failed to initialize physics system!" << std::endl;
        return -1;
    }
    
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
        
        if (!physicsSystem->addParticle(particle)) {
            std::cerr << "Failed to add particle " << i << std::endl;
            break;
        }
    }
    
    // Set gravity
    physicsSystem->setGravity(0.0f, -9.81f, 0.0f);
    
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
        physicsSystem->updatePhysics(deltaTime);
        
        totalTime += deltaTime;
        frameCount++;
        
        // Print statistics every second
        if (totalTime >= 1.0f) {
            auto particles = physicsSystem->getParticles();
            
            // Calculate average height of particles
            float avgHeight = 0.0f;
            float minHeight = particles[0].position[1];
            float maxHeight = particles[0].position[1];
            
            for (const auto& particle : particles) {
                avgHeight += particle.position[1];
                minHeight = std::min(minHeight, particle.position[1]);
                maxHeight = std::max(maxHeight, particle.position[1]);
            }
            avgHeight /= particles.size();
            
            std::cout << "FPS: " << frameCount 
                      << ", Particles: " << particles.size()
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
    
    std::cout << "Simulation ended." << std::endl;
    return 0;
}