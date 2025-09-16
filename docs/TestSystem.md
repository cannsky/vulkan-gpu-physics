# Test System

The Test System provides a comprehensive testing framework for validating the functionality, performance, and reliability of the Titanium Physics Engine and its components.

## Overview

The Test System is a custom-built testing framework designed specifically for the Titanium Physics Engine. It provides structured test management, comprehensive assertion capabilities, and detailed reporting for both unit tests and integration tests across the physics systems.

## Architecture

### Core Testing Framework

The Test System is built around several key components that work together to provide comprehensive testing capabilities:

#### Test Base Class

**Location**: `src/tests/components/tests/Test.h`

```cpp
class Test {
public:
    virtual ~Test() = default;
    
    // Abstract methods that must be implemented by derived classes
    virtual std::string getName() const = 0;
    virtual std::string getClassName() const = 0;
    virtual void run(TestResult& result) = 0;
    
    // Optional setup and teardown methods
    virtual void setUp() {}
    virtual void tearDown() {}
    
    // Utility methods for assertions
protected:
    void assertTrue(bool condition, const std::string& message = "Assertion failed");
    void assertFalse(bool condition, const std::string& message = "Assertion failed");
    void assertEqual(float expected, float actual, float tolerance = 0.001f, const std::string& message = "Values not equal");
    void assertEqual(int expected, int actual, const std::string& message = "Values not equal");
    void assertNotNull(const void* ptr, const std::string& message = "Pointer is null");
    void fail(const std::string& message);
};
```

**Key Features**:
- **Abstract Interface**: Enforces consistent test structure
- **Setup/Teardown**: Optional initialization and cleanup methods
- **Rich Assertions**: Comprehensive assertion methods for different data types
- **Error Handling**: Structured exception-based failure reporting

#### TestResult Structure

**Location**: `src/tests/components/tests/TestResult.h`

```cpp
enum class TestStatus {
    PASSED,
    FAILED,
    SKIPPED
};

struct TestResult {
    std::string testName;
    std::string testClass;
    TestStatus status;
    std::string message;
    std::chrono::milliseconds duration;
    
    TestResult(const std::string& name, const std::string& className);
    
    bool isPassed() const { return status == TestStatus::PASSED; }
    bool isFailed() const { return status == TestStatus::FAILED; }
    bool isSkipped() const { return status == TestStatus::SKIPPED; }
    
    void markPassed(const std::string& msg = "");
    void markFailed(const std::string& msg);
    void markSkipped(const std::string& msg = "");
    void setDuration(std::chrono::milliseconds dur);
};
```

**Result Tracking**:
- **Status Management**: Clear pass/fail/skip status tracking
- **Timing Information**: Performance measurement for each test
- **Detailed Messages**: Rich error and success messaging
- **Metadata**: Test name and class information for organization

### Test Manager

**Purpose**: Central coordinator for test execution and reporting.

**Location**: `src/tests/components/tests/TestManager.h`

```cpp
struct TestSummary {
    int totalTests = 0;
    int passedTests = 0;
    int failedTests = 0;
    int skippedTests = 0;
    std::chrono::milliseconds totalDuration{0};
    std::vector<TestResult> results;
    
    double getPassRate() const {
        return totalTests > 0 ? (static_cast<double>(passedTests) / totalTests) * 100.0 : 0.0;
    }
    
    bool allTestsPassed() const {
        return failedTests == 0 && totalTests > 0;
    }
};

class TestManager : public BaseManager {
public:
    static TestManager& getInstance();
    
    // Test registration and management
    void registerTest(std::unique_ptr<Test> test);
    void clearTests();
    size_t getTestCount() const;
    
    // Test execution
    TestSummary runAllTests();
    TestResult runSingleTest(const std::string& testName);
    TestSummary runTestsByClass(const std::string& className);
    
    // Reporting
    void printSummary(const TestSummary& summary);
    void printDetailedResults(const TestSummary& summary);
    void generateHTMLReport(const TestSummary& summary, const std::string& filename);
    
    // Configuration
    void setVerbose(bool verbose);
    void setTimeout(std::chrono::milliseconds timeout);
    void setContinueOnFailure(bool continueOnFailure);
};
```

#### Test Execution Pipeline

```cpp
TestSummary TestManager::runAllTests() {
    TestSummary summary;
    summary.totalTests = tests.size();
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (auto& test : tests) {
        TestResult result(test->getName(), test->getClassName());
        
        try {
            // Setup phase
            test->setUp();
            
            // Execution phase with timing
            auto testStart = std::chrono::high_resolution_clock::now();
            test->run(result);
            auto testEnd = std::chrono::high_resolution_clock::now();
            
            result.setDuration(std::chrono::duration_cast<std::chrono::milliseconds>(testEnd - testStart));
            
            // Teardown phase
            test->tearDown();
            
            if (result.isPassed()) {
                summary.passedTests++;
                if (verbose) {
                    std::cout << "✓ PASSED: " << result.testName << std::endl;
                }
            }
            
        } catch (const std::exception& e) {
            result.markFailed(e.what());
            summary.failedTests++;
            
            std::cout << "✗ FAILED: " << result.testName << " - " << e.what() << std::endl;
            
            if (!continueOnFailure) {
                break;
            }
        }
        
        summary.results.push_back(result);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    summary.totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    return summary;
}
```

## Test Categories

### Physics System Tests

#### CPU Physics Tests

**RigidBody Tests** (`src/tests/components/tests/tests/RigidBodyTests.h`):

```cpp
class RigidBodyCreationTest : public Test {
public:
    std::string getName() const override { return "RigidBodyCreationTest"; }
    std::string getClassName() const override { return "RigidBodyTests"; }
    
    void run(TestResult& result) override {
        // Test rigidbody creation with various parameters
        cpu_physics::CPUPhysicsEngine engine;
        assertTrue(engine.initialize(10), "Engine initialization failed");
        
        // Test basic creation
        uint32_t entityId = engine.createRigidBody(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0);
        assertTrue(entityId != 0, "Failed to create rigidbody");
        
        // Test component verification
        RigidBodyComponent* rb = engine.getRigidBody(entityId);
        assertNotNull(rb, "RigidBody component not found");
        assertEqual(1.0f, rb->physics.mass, 0.001f, "Mass not set correctly");
        
        result.markPassed("RigidBody creation and component verification successful");
    }
};

class RigidBodyMassCalculationTest : public Test {
public:
    std::string getName() const override { return "RigidBodyMassCalculationTest"; }
    std::string getClassName() const override { return "RigidBodyTests"; }
    
    void run(TestResult& result) override {
        // Test mass and inverse mass calculations
        cpu_physics::CPUPhysicsEngine engine;
        engine.initialize(10);
        
        // Test dynamic body
        uint32_t dynamicId = engine.createRigidBody(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 2.5f, 0);
        RigidBodyComponent* dynamic = engine.getRigidBody(dynamicId);
        assertEqual(2.5f, dynamic->physics.mass, 0.001f, "Dynamic mass incorrect");
        assertEqual(1.0f / 2.5f, dynamic->physics.invMass, 0.001f, "Dynamic inverse mass incorrect");
        
        // Test static body
        uint32_t staticId = engine.createRigidBody(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0);
        RigidBodyComponent* staticRB = engine.getRigidBody(staticId);
        assertEqual(0.0f, staticRB->physics.mass, 0.001f, "Static mass incorrect");
        assertEqual(0.0f, staticRB->physics.invMass, 0.001f, "Static inverse mass should be 0");
        
        result.markPassed("Mass calculations verified for dynamic and static bodies");
    }
};
```

**Collision Tests** (`src/tests/components/tests/tests/CollisionTests.h`):

```cpp
class SphereCollisionDetectionTest : public Test {
public:
    std::string getName() const override { return "SphereCollisionDetectionTest"; }
    std::string getClassName() const override { return "CollisionTests"; }
    
    void run(TestResult& result) override {
        // Test collision detection between overlapping rigidbodies
        cpu_physics::CPUPhysicsEngine engine;
        engine.initialize(10);
        
        // Create two overlapping boxes
        uint32_t box1 = engine.createRigidBody(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, 1.0f, 0);
        uint32_t box2 = engine.createRigidBody(1.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f, 1.0f, 0);
        
        // Run physics step to detect collision
        engine.updatePhysics(0.016f);
        
        // Verify collision was detected (check separation)
        RigidBodyComponent* rb1 = engine.getRigidBody(box1);
        RigidBodyComponent* rb2 = engine.getRigidBody(box2);
        
        float distance = std::abs(rb1->transform.position[0] - rb2->transform.position[0]);
        assertTrue(distance >= 2.0f, "Objects should be separated after collision");
        
        result.markPassed("Collision detection and separation verified");
    }
};

class MaterialPropertiesTest : public Test {
public:
    std::string getName() const override { return "MaterialPropertiesTest"; }
    std::string getClassName() const override { return "CollisionTests"; }
    
    void run(TestResult& result) override {
        // Test material properties affect collision response
        cpu_physics::CPUPhysicsEngine engine;
        engine.initialize(10);
        
        // Create bouncy ball (high restitution)
        uint32_t bouncyBall = engine.createRigidBody(0.0f, 10.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0);
        RigidBodyComponent* bouncy = engine.getRigidBody(bouncyBall);
        bouncy->physics.restitution = 0.9f; // Very bouncy
        bouncy->physics.velocity[1] = -5.0f; // Downward velocity
        
        // Create ground
        uint32_t ground = engine.createRigidBody(0.0f, -5.0f, 0.0f, 10.0f, 1.0f, 10.0f, 0.0f, 0);
        
        // Simulate collision
        for (int i = 0; i < 60; ++i) { // 1 second at 60 FPS
            engine.updatePhysics(0.016f);
        }
        
        // Ball should bounce back up
        assertTrue(bouncy->physics.velocity[1] > 0.0f, "Ball should bounce upward");
        assertTrue(bouncy->transform.position[1] > 0.0f, "Ball should be above ground");
        
        result.markPassed("Material properties affect collision response correctly");
    }
};
```

#### GPU Physics Tests

```cpp
#ifdef VULKAN_AVAILABLE
class ParticleSystemTest : public Test {
public:
    std::string getName() const override { return "ParticleSystemTest"; }
    std::string getClassName() const override { return "GPUPhysicsTests"; }
    
    void run(TestResult& result) override {
        // Test GPU particle system functionality
        auto vulkanContext = std::make_shared<VulkanContext>();
        if (!vulkanContext->initialize()) {
            result.markSkipped("Vulkan not available for GPU physics tests");
            return;
        }
        
        gpu_physics::GPUPhysicsEngine engine(vulkanContext);
        assertTrue(engine.initialize(1000), "GPU physics engine initialization failed");
        
        // Add particles
        for (int i = 0; i < 100; ++i) {
            bool success = engine.addParticle(
                i * 0.1f, 10.0f, 0.0f,  // position
                0.0f, 0.0f, 0.0f,       // velocity
                1.0f                    // mass
            );
            assertTrue(success, "Failed to add particle " + std::to_string(i));
        }
        
        assertEqual(100, (int)engine.getParticleCount(), "Incorrect particle count");
        
        // Run simulation
        engine.updatePhysics(0.016f);
        
        // Verify particles moved due to gravity
        auto particles = engine.getParticles();
        for (const auto& particle : particles) {
            assertTrue(particle.position[1] < 10.0f, "Particle should have fallen due to gravity");
        }
        
        result.markPassed("GPU particle system creation and simulation verified");
    }
};

class ComputeShaderTest : public Test {
public:
    std::string getName() const override { return "ComputeShaderTest"; }
    std::string getClassName() const override { return "GPUPhysicsTests"; }
    
    void run(TestResult& result) override {
        // Test compute shader functionality
        auto vulkanContext = std::make_shared<VulkanContext>();
        if (!vulkanContext->initialize()) {
            result.markSkipped("Vulkan not available for compute shader tests");
            return;
        }
        
        // Test buffer manager
        BufferManager bufferManager(vulkanContext);
        assertTrue(bufferManager.initialize(), "Buffer manager initialization failed");
        
        // Test compute pipeline
        ComputePipeline pipeline(vulkanContext);
        assertTrue(pipeline.initialize("shaders/physics.comp.spv"), "Compute pipeline initialization failed");
        
        // Test buffer creation
        VkBuffer particleBuffer = bufferManager.createStorageBuffer(1000 * sizeof(Particle));
        assertTrue(particleBuffer != VK_NULL_HANDLE, "Failed to create particle storage buffer");
        
        VkBuffer uniformBuffer = bufferManager.createUniformBuffer(sizeof(SimulationParams));
        assertTrue(uniformBuffer != VK_NULL_HANDLE, "Failed to create uniform buffer");
        
        result.markPassed("Compute shader components verified");
    }
};
#endif
```

### Integration Tests

#### Hybrid System Tests

```cpp
class HybridPhysicsTest : public Test {
public:
    std::string getName() const override { return "HybridPhysicsTest"; }
    std::string getClassName() const override { return "IntegrationTests"; }
    
    void run(TestResult& result) override {
        // Test integration between CPU and GPU physics
        PhysicsEngine engine;
        assertTrue(engine.initialize(100, 50), "Hybrid physics engine initialization failed");
        
        // Create CPU rigidbody
        uint32_t rigidBodyId = engine.createRigidBody(0.0f, 10.0f, 0.0f, 2.0f, 2.0f, 2.0f, 1.0f, 0);
        assertTrue(rigidBodyId != 0, "Failed to create rigidbody");
        
#ifdef VULKAN_AVAILABLE
        // Add GPU particles
        for (int i = 0; i < 10; ++i) {
            bool success = engine.addParticle(i * 2.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
            assertTrue(success, "Failed to add particle " + std::to_string(i));
        }
#endif
        
        // Run hybrid simulation
        for (int i = 0; i < 60; ++i) {
            engine.updatePhysics(0.016f);
        }
        
        // Verify both systems updated
        RigidBodyComponent* rb = engine.getRigidBody(rigidBodyId);
        assertNotNull(rb, "RigidBody not found after simulation");
        assertTrue(rb->transform.position[1] < 10.0f, "RigidBody should have fallen");
        
#ifdef VULKAN_AVAILABLE
        auto particles = engine.getParticles();
        assertTrue(particles.size() == 10, "Incorrect particle count after simulation");
        for (const auto& particle : particles) {
            assertTrue(particle.position[1] < 15.0f, "Particles should have fallen");
        }
#endif
        
        result.markPassed("Hybrid physics simulation verified");
    }
};
```

### Performance Tests

```cpp
class PerformanceBenchmark : public Test {
public:
    std::string getName() const override { return "PerformanceBenchmark"; }
    std::string getClassName() const override { return "PerformanceTests"; }
    
    void run(TestResult& result) override {
        // Benchmark physics performance
        PhysicsEngine engine;
        engine.initialize(1000, 100);
        
        // Create many rigidbodies
        std::vector<uint32_t> rigidBodies;
        for (int i = 0; i < 100; ++i) {
            uint32_t id = engine.createRigidBody(
                (i % 10) * 2.0f, 20.0f + (i / 10) * 2.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 0
            );
            rigidBodies.push_back(id);
        }
        
#ifdef VULKAN_AVAILABLE
        // Add many particles
        for (int i = 0; i < 1000; ++i) {
            engine.addParticle(
                (rand() % 20) - 10.0f, 30.0f, (rand() % 20) - 10.0f,
                0.0f, 0.0f, 0.0f, 0.1f
            );
        }
#endif
        
        // Benchmark simulation
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int frame = 0; frame < 300; ++frame) { // 5 seconds at 60 FPS
            engine.updatePhysics(0.016f);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Verify reasonable performance (should complete in reasonable time)
        assertTrue(duration.count() < 10000, "Simulation took too long: " + std::to_string(duration.count()) + "ms");
        
        result.markPassed("Performance benchmark completed in " + std::to_string(duration.count()) + "ms");
    }
};
```

## Test Execution Models

### Simple Test Framework

**Location**: `src/tests/test.cpp`

The project also includes a simplified test framework for quick validation:

```cpp
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
                cpu_physics::CPUPhysicsEngine cpuEngine;
                assert(cpuEngine.initialize(10));
                std::cout << "✓ PASSED: CPU Physics Engine initialization" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: CPU Physics Engine initialization - " << e.what() << std::endl;
            }
            
            // Test 2: RigidBody creation
            std::cout << "\n[Test 2] RigidBody creation..." << std::endl;
            totalTests++;
            try {
                cpu_physics::CPUPhysicsEngine cpuEngine;
                cpuEngine.initialize(10);
                uint32_t entityId = cpuEngine.createRigidBody(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0);
                assert(entityId != 0);
                RigidBodyComponent* rb = cpuEngine.getRigidBody(entityId);
                assert(rb != nullptr);
                std::cout << "✓ PASSED: RigidBody creation" << std::endl;
                passedTests++;
            } catch (const std::exception& e) {
                std::cout << "✗ FAILED: RigidBody creation - " << e.what() << std::endl;
            }
            
            // Additional tests...
            
            // Test Summary
            std::cout << "\n=== Test Summary ===" << std::endl;
            std::cout << "Total tests: " << totalTests << std::endl;
            std::cout << "Passed: " << passedTests << std::endl;
            std::cout << "Failed: " << (totalTests - passedTests) << std::endl;
            std::cout << "Success rate: " << (passedTests * 100 / totalTests) << "%" << std::endl;
            
            if (passedTests == totalTests) {
                std::cout << "\n🎉 All tests passed!" << std::endl;
                return 0;
            } else {
                std::cout << "\n❌ Some tests failed!" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cout << "Fatal error during testing: " << e.what() << std::endl;
            return -1;
        }
    }
};
```

### Comprehensive Test Runner

**Location**: `src/tests/components/tests/tests/test_runner.cpp`

```cpp
int main() {
    try {
        // Configure logger for testing
        Logger::getInstance().setLogLevel(LogLevel::INFO);
        Logger::getInstance().enableCategory(LogCategory::COLLISION);
        Logger::getInstance().enableCategory(LogCategory::RIGIDBODY);
        Logger::getInstance().enableCategory(LogCategory::GENERAL);
        Logger::getInstance().enableConsoleOutput(true);
        
        // Get test manager instance and initialize
        TestManager& testManager = TestManager::getInstance();
        if (!testManager.initialize()) {
            std::cerr << "Failed to initialize TestManager" << std::endl;
            return -1;
        }
        
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
        
        // Print summary
        testManager.printSummary(summary);
        
        // Print detailed results if there were failures
        if (summary.failedTests > 0) {
            testManager.printDetailedResults(summary);
        }
        
        // Cleanup test manager
        testManager.cleanup();
        
        // Return appropriate exit code
        return summary.allTestsPassed() ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error during testing: " << e.what() << std::endl;
        return -1;
    }
}
```

## Test Configuration and Customization

### Test Manager Configuration

```cpp
class TestConfiguration {
public:
    // Execution settings
    bool verbose = false;
    bool continueOnFailure = true;
    std::chrono::milliseconds timeout{30000}; // 30 second default timeout
    
    // Reporting settings
    bool generateHTMLReport = false;
    std::string htmlReportPath = "test_results.html";
    bool logToFile = false;
    std::string logFilePath = "test_log.txt";
    
    // Test filtering
    std::vector<std::string> includeClasses;
    std::vector<std::string> excludeClasses;
    std::vector<std::string> includeTests;
    std::vector<std::string> excludeTests;
    
    // Performance settings
    bool enablePerformanceTests = false;
    bool enableMemoryLeakDetection = false;
    int performanceIterations = 1;
};
```

### Custom Test Assertions

```cpp
// Extended assertion library for physics-specific tests
class PhysicsAssertions {
public:
    static void assertVector3Equal(const float expected[3], const float actual[3], 
                                 float tolerance = 0.001f, const std::string& message = "") {
        for (int i = 0; i < 3; ++i) {
            if (std::abs(expected[i] - actual[i]) > tolerance) {
                throw std::runtime_error("Vector3 assertion failed: " + message + 
                                       " at component " + std::to_string(i) +
                                       " (expected: " + std::to_string(expected[i]) + 
                                       ", actual: " + std::to_string(actual[i]) + ")");
            }
        }
    }
    
    static void assertPhysicsBodyValid(const RigidBodyComponent* body, const std::string& message = "") {
        if (!body) {
            throw std::runtime_error("Physics body assertion failed: " + message + " (body is null)");
        }
        
        // Check for valid mass
        if (body->physics.mass < 0.0f) {
            throw std::runtime_error("Physics body assertion failed: " + message + " (negative mass)");
        }
        
        // Check for NaN values
        for (int i = 0; i < 3; ++i) {
            if (std::isnan(body->transform.position[i]) || std::isnan(body->physics.velocity[i])) {
                throw std::runtime_error("Physics body assertion failed: " + message + " (NaN values detected)");
            }
        }
    }
    
    static void assertCollisionOccurred(const RigidBodyComponent* body1, const RigidBodyComponent* body2,
                                       float deltaTime, const std::string& message = "") {
        // Check if bodies are separated (indicating collision response occurred)
        float distance = 0.0f;
        for (int i = 0; i < 3; ++i) {
            float diff = body1->transform.position[i] - body2->transform.position[i];
            distance += diff * diff;
        }
        distance = std::sqrt(distance);
        
        float minSeparation = (body1->collider.width + body2->collider.width) * 0.5f;
        if (distance < minSeparation) {
            throw std::runtime_error("Collision assertion failed: " + message + " (bodies still overlapping)");
        }
    }
};
```

## Continuous Integration Support

### Automated Test Execution

```bash
#!/bin/bash
# CI test script
set -e

echo "Building test targets..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DVULKAN_AVAILABLE=OFF
make -j$(nproc)

echo "Running simple tests..."
./test-titanium-physics

echo "Running comprehensive tests..."
./physics-tests

echo "Running legacy tests..."
./legacy-physics-tests

echo "All tests completed successfully!"
```

### Test Report Generation

```cpp
void TestManager::generateHTMLReport(const TestSummary& summary, const std::string& filename) {
    std::ofstream htmlFile(filename);
    
    htmlFile << "<!DOCTYPE html>\n<html>\n<head>\n";
    htmlFile << "<title>Physics Engine Test Results</title>\n";
    htmlFile << "<style>\n";
    htmlFile << ".passed { color: green; }\n";
    htmlFile << ".failed { color: red; }\n";
    htmlFile << ".skipped { color: orange; }\n";
    htmlFile << "table { border-collapse: collapse; width: 100%; }\n";
    htmlFile << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    htmlFile << "</style>\n</head>\n<body>\n";
    
    htmlFile << "<h1>Physics Engine Test Results</h1>\n";
    htmlFile << "<h2>Summary</h2>\n";
    htmlFile << "<p>Total Tests: " << summary.totalTests << "</p>\n";
    htmlFile << "<p>Passed: <span class='passed'>" << summary.passedTests << "</span></p>\n";
    htmlFile << "<p>Failed: <span class='failed'>" << summary.failedTests << "</span></p>\n";
    htmlFile << "<p>Skipped: <span class='skipped'>" << summary.skippedTests << "</span></p>\n";
    htmlFile << "<p>Pass Rate: " << std::fixed << std::setprecision(1) << summary.getPassRate() << "%</p>\n";
    htmlFile << "<p>Total Duration: " << summary.totalDuration.count() << "ms</p>\n";
    
    htmlFile << "<h2>Detailed Results</h2>\n";
    htmlFile << "<table>\n<tr><th>Test Name</th><th>Class</th><th>Status</th><th>Duration</th><th>Message</th></tr>\n";
    
    for (const auto& result : summary.results) {
        std::string statusClass = result.isPassed() ? "passed" : (result.isFailed() ? "failed" : "skipped");
        std::string statusText = result.isPassed() ? "PASSED" : (result.isFailed() ? "FAILED" : "SKIPPED");
        
        htmlFile << "<tr>\n";
        htmlFile << "<td>" << result.testName << "</td>\n";
        htmlFile << "<td>" << result.testClass << "</td>\n";
        htmlFile << "<td class='" << statusClass << "'>" << statusText << "</td>\n";
        htmlFile << "<td>" << result.duration.count() << "ms</td>\n";
        htmlFile << "<td>" << result.message << "</td>\n";
        htmlFile << "</tr>\n";
    }
    
    htmlFile << "</table>\n</body>\n</html>\n";
    htmlFile.close();
}
```

## Design Benefits

### Comprehensive Coverage
- **Unit Tests**: Individual component validation
- **Integration Tests**: System interaction verification
- **Performance Tests**: Performance regression detection
- **Cross-Platform Tests**: Vulkan availability handling

### Developer Productivity
- **Rich Assertions**: Physics-specific assertion methods
- **Clear Reporting**: Detailed test results and error messages
- **Fast Execution**: Efficient test execution with minimal overhead
- **Easy Extension**: Simple framework for adding new tests

### Quality Assurance
- **Automated Validation**: Continuous integration support
- **Regression Prevention**: Comprehensive test coverage prevents breaking changes
- **Performance Monitoring**: Performance tests detect optimization regressions
- **Documentation**: Tests serve as executable documentation

The Test System provides a robust foundation for validating the Titanium Physics Engine, ensuring reliability, performance, and correctness across all components and usage scenarios.