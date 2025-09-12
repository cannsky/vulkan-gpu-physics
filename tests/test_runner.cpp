#include "framework/TestManager.h"
#include "RigidBodyTests.h"
#include "CollisionTests.h"
#include "LoggerTests.h"
#include "../src/logger/Logger.h"
#include <memory>
#include <iostream>

int main() {
    try {
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
        
        // Return appropriate exit code
        return summary.allTestsPassed() ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Test execution failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test execution failed with unknown exception" << std::endl;
        return 1;
    }
}