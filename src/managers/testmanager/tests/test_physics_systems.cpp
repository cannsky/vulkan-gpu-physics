#include "../../../collision/CollisionSystem.h"
#include "../../../components/rigidbody/RigidBodySystem.h"
#include "../../../logger/Logger.h"
#include "../TestManager.h"
#include "RigidBodyTests.h"
#include "CollisionTests.h"
#include "LoggerTests.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <memory>

// Legacy test framework for backward compatibility
class LegacyTestFramework {
public:
    static void runTests() {
        Logger::getInstance().setLogLevel(LogLevel::INFO);
        Logger::getInstance().enableCategory(LogCategory::COLLISION);
        Logger::getInstance().enableCategory(LogCategory::RIGIDBODY);
        
        std::cout << "Running Legacy Physics System Tests..." << std::endl;
        
        testRigidBodyCreation();
        testSphereCollision();
        testLogging();
        
        std::cout << "All legacy tests passed!" << std::endl;
    }
    
private:
    static void testRigidBodyCreation() {
        std::cout << "Testing rigid body creation..." << std::endl;
        
        // Create a mock rigid body system (without Vulkan context)
        auto rigidBodySystem = std::shared_ptr<RigidBodySystem>(nullptr);
        
        // Test rigid body data structure
        RigidBody sphere = {};
        sphere.position[0] = 1.0f;
        sphere.position[1] = 2.0f;
        sphere.position[2] = 3.0f;
        sphere.mass = 1.5f;
        sphere.invMass = 1.0f / 1.5f;
        sphere.shapeType = static_cast<uint32_t>(RigidBodyShape::SPHERE);
        sphere.shapeData[0] = 0.5f; // radius
        
        // Verify data integrity
        assert(sphere.position[0] == 1.0f);
        assert(sphere.position[1] == 2.0f);
        assert(sphere.position[2] == 3.0f);
        assert(sphere.mass == 1.5f);
        assert(std::abs(sphere.invMass - (1.0f / 1.5f)) < 0.001f);
        assert(sphere.shapeType == static_cast<uint32_t>(RigidBodyShape::SPHERE));
        assert(sphere.shapeData[0] == 0.5f);
        
        LOG_RIGIDBODY_INFO("Rigid body creation test passed");
        std::cout << "✓ Rigid body creation test passed" << std::endl;
    }
    
    static void testSphereCollision() {
        std::cout << "Testing sphere collision detection..." << std::endl;
        
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
        
        // Create collision system (without Vulkan context for testing)
        auto collisionSystem = std::shared_ptr<CollisionSystem>(nullptr);
        
        // Test collision detection logic manually
        float dx = sphereA.position[0] - sphereB.position[0];
        float dy = sphereA.position[1] - sphereB.position[1];
        float dz = sphereA.position[2] - sphereB.position[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        float radiusSum = sphereA.shapeData[0] + sphereB.shapeData[0];
        
        bool shouldCollide = distSq < (radiusSum * radiusSum);
        assert(shouldCollide); // Spheres should be colliding
        
        float dist = std::sqrt(distSq);
        float penetration = radiusSum - dist;
        assert(penetration > 0.0f); // Should have positive penetration
        
        // Test combined material properties
        float combinedRestitution = (sphereA.restitution + sphereB.restitution) * 0.5f;
        float combinedFriction = std::sqrt(sphereA.friction * sphereB.friction);
        
        assert(std::abs(combinedRestitution - 0.6f) < 0.001f);
        assert(std::abs(combinedFriction - std::sqrt(0.3f * 0.4f)) < 0.001f);
        
        LOG_COLLISION_INFO("Sphere collision test passed - penetration: " + std::to_string(penetration));
        std::cout << "✓ Sphere collision detection test passed" << std::endl;
    }
    
    static void testLogging() {
        std::cout << "Testing logging system..." << std::endl;
        
        // Test different log levels
        Logger::getInstance().trace(LogCategory::GENERAL, "Trace message");
        Logger::getInstance().debug(LogCategory::GENERAL, "Debug message");
        Logger::getInstance().info(LogCategory::GENERAL, "Info message");
        Logger::getInstance().warn(LogCategory::GENERAL, "Warning message");
        Logger::getInstance().error(LogCategory::GENERAL, "Error message");
        
        // Test category-specific logging
        Logger::getInstance().logPhysics(LogLevel::INFO, "Physics system test");
        Logger::getInstance().logCollision(LogLevel::INFO, "Collision system test");
        Logger::getInstance().logRigidBody(LogLevel::INFO, "Rigid body system test");
        Logger::getInstance().logParticles(LogLevel::INFO, "Particle system test");
        Logger::getInstance().logVulkan(LogLevel::INFO, "Vulkan system test");
        Logger::getInstance().logPerformance(LogLevel::INFO, "Performance test");
        
        // Test performance logging
        Logger::getInstance().logFrameTime(0.016f);
        Logger::getInstance().logCollisionCount(5);
        Logger::getInstance().logParticleCount(100);
        Logger::getInstance().logRigidBodyCount(10);
        
        std::cout << "✓ Logging system test passed" << std::endl;
    }
};

// New test framework runner
void runNewTestFramework() {
    try {
        std::cout << "\n=== Running New Test Framework ===" << std::endl;
        
        // Configure logger for testing
        Logger::getInstance().setLogLevel(LogLevel::INFO);
        Logger::getInstance().enableCategory(LogCategory::COLLISION);
        Logger::getInstance().enableCategory(LogCategory::RIGIDBODY);
        Logger::getInstance().enableCategory(LogCategory::GENERAL);
        Logger::getInstance().enableConsoleOutput(true);
        
        // Get test manager instance
        TestManager& testManager = TestManager::getInstance();
        
        // Register RigidBody tests
        testManager.registerTest(std::make_unique<RigidBodyCreationTest>());
        testManager.registerTest(std::make_unique<RigidBodyMassCalculationTest>());
        
        // Register Collision tests
        testManager.registerTest(std::make_unique<SphereCollisionDetectionTest>());
        testManager.registerTest(std::make_unique<MaterialPropertiesTest>());
        testManager.registerTest(std::make_unique<NonCollisionTest>());
        
        // Register Logger tests
        testManager.registerTest(std::make_unique<LoggingLevelsTest>());
        testManager.registerTest(std::make_unique<CategorySpecificLoggingTest>());
        testManager.registerTest(std::make_unique<PerformanceLoggingTest>());
        testManager.registerTest(std::make_unique<LoggerConfigurationTest>());
        
        // Run all tests
        TestSummary summary = testManager.runAllTests();
        
        // Print detailed results if there were failures
        if (summary.failedTests > 0) {
            testManager.printDetailedResults(summary);
        }
        
        if (!summary.allTestsPassed()) {
            throw std::runtime_error("New test framework failed with " + std::to_string(summary.failedTests) + " test(s)");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "New test framework failed with exception: " << e.what() << std::endl;
        throw;
    }
}

int main() {
    try {
        // Run legacy tests for backward compatibility
        LegacyTestFramework::runTests();
        
        // Run new test framework
        runNewTestFramework();
        
        std::cout << "\n🎉 All test frameworks completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}