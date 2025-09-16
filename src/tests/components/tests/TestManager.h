#pragma once

#include "../../../PhysicsEngine/managers/BaseManager.h"
#include "Test.h"
#include "TestResult.h"
#include <vector>
#include <memory>
#include <string>

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
    
    // BaseManager interface
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    
    // Test registration
    void registerTest(std::unique_ptr<Test> test);
    
    // Test execution
    TestSummary runAllTests();
    TestResult runSingleTest(Test* test);
    
    // Utility methods
    void clear();
    size_t getTestCount() const;
    void printTestSummary(const TestSummary& summary) const;
    void printDetailedResults(const TestSummary& summary) const;

private:
    TestManager() = default;
    ~TestManager() = default;
    
    // Non-copyable and non-movable (inherited from BaseManager)
    
    std::vector<std::unique_ptr<Test>> tests;
    bool initialized = false;
    
    void printColoredStatus(TestStatus status) const;
    std::string formatDuration(std::chrono::milliseconds duration) const;
};