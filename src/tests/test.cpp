#include "../tests/components/tests/TestManager.h"
#include "../PhysicsEngine/managers/logmanager/Logger.h"
#include "../PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.h"
#include "../PhysicsEngine/PhysicsEngine.h"
#include <memory>
#include <iostream>
#include <cassert>
#include <cmath>

// Simple consolidated test framework that doesn't depend on complex test classes
class SimpleTestFramework {
public:
    static int runAllTests() {
        try {
            // Configure logger for testing
            Logger::getInstance().setLogLevel(LogLevel::INFO);
            Logger::getInstance().enableCategory(LogCategory::COLLISION);
            Logger::getInstance().enableCategory(LogCategory::RIGIDBODY);
            Logger::getInstance().enableCategory(LogCategory::GENERAL);
            Logger::getInstance().enableCategory(LogCategory::PHYSICS);
            Logger::getInstance().enableConsoleOutput(true);
            
            std::cout << "=== Titanium Physics Engine - Simplified Test Suite ===" << std::endl;
            std::cout << "Running essential physics system tests..." << std::endl;
            
            int passedTests = 0;
            int totalTests = 0;
            
            // Test 1: CPU Physics Engine initialization
            std::cout << "\n[Test 1] CPU Physics Engine initialization..." << std::endl;
            totalTests++;
            try {
                auto cpuEngine = std::make_unique<cpu_physics::CPUPhysicsEngine>();
                assert(cpuEngine->initialize(10));
                std::cout << "✓ PASSED: CPU Physics Engine initialization" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: CPU Physics Engine initialization - " << e.what() << std::endl;
            }
            
            // Test 2: RigidBody creation
            std::cout << "\n[Test 2] RigidBody creation..." << std::endl;
            totalTests++;
            try {
                auto cpuEngine = std::make_unique<cpu_physics::CPUPhysicsEngine>();
                cpuEngine->initialize(10);
                uint32_t rigidBodyId = cpuEngine->createRigidBody(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0);
                assert(rigidBodyId != 0);
                std::cout << "✓ PASSED: RigidBody creation" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: RigidBody creation - " << e.what() << std::endl;
            }
            
            // Test 3: Physics simulation step
            std::cout << "\n[Test 3] Physics simulation step..." << std::endl;
            totalTests++;
            try {
                auto cpuEngine = std::make_unique<cpu_physics::CPUPhysicsEngine>();
                cpuEngine->initialize(10);
                cpuEngine->createRigidBody(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0);
                cpuEngine->updatePhysics(0.016f); // 60 FPS step
                std::cout << "✓ PASSED: Physics simulation step" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: Physics simulation step - " << e.what() << std::endl;
            }
            
            // Test 4: Physics Engine (hybrid) initialization
            std::cout << "\n[Test 4] Hybrid Physics Engine initialization..." << std::endl;
            totalTests++;
            try {
                PhysicsEngine physicsEngine;
                assert(physicsEngine.initialize(0, 10)); // CPU-only
                std::cout << "✓ PASSED: Hybrid Physics Engine initialization" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: Hybrid Physics Engine initialization - " << e.what() << std::endl;
            }
            
            // Test 5: Physics layer creation
            std::cout << "\n[Test 5] Physics layer creation..." << std::endl;
            totalTests++;
            try {
                PhysicsEngine physicsEngine;
                physicsEngine.initialize(0, 10);
                uint32_t layerId = physicsEngine.createPhysicsLayer("TestLayer");
                assert(layerId != 0);
                std::cout << "✓ PASSED: Physics layer creation" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: Physics layer creation - " << e.what() << std::endl;
            }
            
            // Test 6: Logger functionality
            std::cout << "\n[Test 6] Logger functionality..." << std::endl;
            totalTests++;
            try {
                Logger::getInstance().info(LogCategory::GENERAL, "Test log message");
                Logger::getInstance().debug(LogCategory::PHYSICS, "Debug test message");
                std::cout << "✓ PASSED: Logger functionality" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: Logger functionality - " << e.what() << std::endl;
            }
            
            std::cout << "\n=== Test Summary ===" << std::endl;
            std::cout << "Total tests: " << totalTests << std::endl;
            std::cout << "Passed: " << passedTests << std::endl;
            std::cout << "Failed: " << (totalTests - passedTests) << std::endl;
            std::cout << "Success rate: " << (passedTests * 100.0 / totalTests) << "%" << std::endl;
            
            if (passedTests == totalTests) {
                std::cout << "\n🎉 All tests passed!" << std::endl;
                return 0;
            } else {
                std::cout << "\n❌ Some tests failed!" << std::endl;
                return 1;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Test execution failed with exception: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Test execution failed with unknown exception" << std::endl;
            return 1;
        }
    }
};

int main() {
    return SimpleTestFramework::runAllTests();
}