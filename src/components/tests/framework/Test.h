#pragma once

#include "TestResult.h"
#include <string>
#include <stdexcept>

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
    void assertTrue(bool condition, const std::string& message = "Assertion failed") {
        if (!condition) {
            throw std::runtime_error("Assert True failed: " + message);
        }
    }
    
    void assertFalse(bool condition, const std::string& message = "Assertion failed") {
        if (condition) {
            throw std::runtime_error("Assert False failed: " + message);
        }
    }
    
    void assertEqual(float expected, float actual, float tolerance = 0.001f, const std::string& message = "Values not equal") {
        if (std::abs(expected - actual) > tolerance) {
            throw std::runtime_error("Assert Equal failed: " + message + 
                                   " (expected: " + std::to_string(expected) + 
                                   ", actual: " + std::to_string(actual) + ")");
        }
    }
    
    void assertEqual(int expected, int actual, const std::string& message = "Values not equal") {
        if (expected != actual) {
            throw std::runtime_error("Assert Equal failed: " + message + 
                                   " (expected: " + std::to_string(expected) + 
                                   ", actual: " + std::to_string(actual) + ")");
        }
    }
    
    void assertNotNull(const void* ptr, const std::string& message = "Pointer is null") {
        if (ptr == nullptr) {
            throw std::runtime_error("Assert Not Null failed: " + message);
        }
    }
    
    void fail(const std::string& message) {
        throw std::runtime_error("Test failed: " + message);
    }
};