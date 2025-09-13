# Test Framework Documentation

## Overview

The Vulkan GPU Physics project now includes a comprehensive test framework that provides:

- Abstract `Test` base class for creating individual tests
- `TestManager` for discovering and running all registered tests  
- Detailed test reporting with timing and colorized output
- CTest integration for CI/CD pipelines
- Backward compatibility with existing tests

## Test Framework Components

### Core Classes

#### `Test` (Abstract Base Class)
- Pure virtual base class that all tests must inherit from
- Provides assertion utilities: `assertTrue()`, `assertEqual()`, `assertFalse()`, etc.
- Includes setup/teardown lifecycle methods

#### `TestResult`
- Stores test outcome (PASSED/FAILED/SKIPPED)
- Captures test duration and error messages
- Provides status checking methods

#### `TestManager` (Singleton)
- Discovers and runs all registered tests
- Provides detailed reporting with colorized output
- Tracks test statistics and timing

### Creating New Tests

```cpp
#include "framework/Test.h"

class MyNewTest : public Test {
public:
    std::string getName() const override {
        return "MyTestName";
    }
    
    std::string getClassName() const override {
        return "MyTestClass";
    }
    
    void run(TestResult& result) override {
        // Your test logic here
        assertTrue(someCondition, "Error message");
        assertEqual(expected, actual, "Values should match");
        
        result.markPassed("Test completed successfully");
    }
    
    // Optional: Override for test setup
    void setUp() override {
        // Initialize test data
    }
    
    // Optional: Override for test cleanup  
    void tearDown() override {
        // Clean up resources
    }
};
```

### Registering Tests

```cpp
#include "framework/TestManager.h"
#include "MyNewTest.h"

int main() {
    TestManager& testManager = TestManager::getInstance();
    
    // Register your test
    testManager.registerTest(std::make_unique<MyNewTest>());
    
    // Run all tests
    TestSummary summary = testManager.runAllTests();
    
    return summary.allTestsPassed() ? 0 : 1;
}
```

## Test Categories

### RigidBodyTests
- `RigidBodyCreation`: Tests creation and initialization of rigid bodies
- `MassCalculation`: Tests mass and inverse mass calculations

### CollisionTests  
- `SphereCollisionDetection`: Tests sphere-sphere collision detection
- `MaterialProperties`: Tests material property combinations
- `NonCollision`: Tests scenarios where objects should not collide

### LoggerTests
- `LoggingLevels`: Tests different log levels (TRACE, DEBUG, INFO, WARN, ERROR)
- `CategorySpecificLogging`: Tests category-based logging
- `PerformanceLogging`: Tests performance metric logging
- `LoggerConfiguration`: Tests logger configuration options

## Running Tests

### Build and Run
```bash
# Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Run tests directly
./physics-tests

# Run through CTest
ctest --build-config Release
```

### Expected Output

```
🧪 Running Physics Engine Tests
=================================
Total tests to run: 9

[  1] PASSED RigidBodyTests::RigidBodyCreation (2ms)
[  2] PASSED RigidBodyTests::MassCalculation (1ms)
[  3] PASSED CollisionTests::SphereCollisionDetection (3ms)
[  4] PASSED CollisionTests::MaterialProperties (1ms)
[  5] PASSED CollisionTests::NonCollision (1ms)
[  6] PASSED LoggerTests::LoggingLevels (5ms)
[  7] PASSED LoggerTests::CategorySpecificLogging (3ms)
[  8] PASSED LoggerTests::PerformanceLogging (2ms)
[  9] PASSED LoggerTests::LoggerConfiguration (1ms)

Test Summary:
=============
Total Tests: 9
Passed: 9
Failed: 0
Skipped: 0
Pass Rate: 100.0%
Total Duration: 19ms

🎉 All tests passed!
```

## CTest Integration

The framework is fully integrated with CTest, fixing the "No tests were found!!!" issue:

```bash
$ ctest --build-config Release
Test project /path/to/build
    Start 1: PhysicsEngineTests
1/1 Test #1: PhysicsEngineTests ...............   Passed    0.02 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.05 sec
```

## Assertion Methods

The `Test` base class provides several assertion methods:

- `assertTrue(condition, message)`: Asserts condition is true
- `assertFalse(condition, message)`: Asserts condition is false  
- `assertEqual(expected, actual, tolerance, message)`: Asserts values are equal (with optional tolerance for floats)
- `assertNotNull(pointer, message)`: Asserts pointer is not null
- `fail(message)`: Explicitly fails the test with a message

## Best Practices

1. **Test Naming**: Use descriptive test names that clearly indicate what is being tested
2. **Test Isolation**: Each test should be independent and not rely on other tests
3. **Clear Assertions**: Use descriptive error messages in assertions
4. **Setup/Teardown**: Use setUp() and tearDown() for resource management
5. **Test Categories**: Group related tests into logical test classes

## Backward Compatibility

The existing `test_physics_systems.cpp` has been updated to:
- Maintain the original legacy test framework for compatibility
- Integrate the new test framework alongside legacy tests
- Demonstrate both testing approaches in a single executable

This ensures that existing workflows continue to work while providing the new enhanced testing capabilities.