#include "TestManager.h"
#include <iostream>
#include <iomanip>
#include <chrono>

// ANSI color codes for terminal output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

TestManager& TestManager::getInstance() {
    static TestManager instance;
    return instance;
}

void TestManager::registerTest(std::unique_ptr<Test> test) {
    tests.push_back(std::move(test));
}

TestSummary TestManager::runAllTests() {
    TestSummary summary;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << BOLD CYAN "🧪 Running Physics Engine Tests" RESET << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "Total tests to run: " << tests.size() << std::endl << std::endl;
    
    for (auto& test : tests) {
        TestResult result = runSingleTest(test.get());
        summary.results.push_back(result);
        summary.totalTests++;
        
        switch (result.status) {
            case TestStatus::PASSED:
                summary.passedTests++;
                break;
            case TestStatus::FAILED:
                summary.failedTests++;
                break;
            case TestStatus::SKIPPED:
                summary.skippedTests++;
                break;
        }
        
        // Print test result immediately
        std::cout << "[" << std::setw(3) << summary.totalTests << "] ";
        printColoredStatus(result.status);
        std::cout << " " << result.testClass << "::" << result.testName;
        std::cout << " (" << formatDuration(result.duration) << ")";
        
        if (!result.message.empty() && result.status != TestStatus::PASSED) {
            std::cout << " - " << result.message;
        }
        std::cout << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    summary.totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << std::endl;
    printTestSummary(summary);
    
    return summary;
}

TestResult TestManager::runSingleTest(Test* test) {
    TestResult result(test->getName(), test->getClassName());
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        test->setUp();
        test->run(result);
        test->tearDown();
        
        // If no exception was thrown and status wasn't explicitly set to FAILED
        if (result.status != TestStatus::FAILED) {
            result.markPassed();
        }
    } catch (const std::exception& e) {
        result.markFailed(e.what());
        try {
            test->tearDown(); // Try to clean up even if test failed
        } catch (...) {
            // Ignore teardown exceptions
        }
    } catch (...) {
        result.markFailed("Unknown exception occurred");
        try {
            test->tearDown();
        } catch (...) {
            // Ignore teardown exceptions
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.setDuration(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime));
    
    return result;
}

void TestManager::clear() {
    tests.clear();
}

size_t TestManager::getTestCount() const {
    return tests.size();
}

void TestManager::printTestSummary(const TestSummary& summary) const {
    std::cout << BOLD "Test Summary:" RESET << std::endl;
    std::cout << "=============" << std::endl;
    
    std::cout << "Total Tests: " << summary.totalTests << std::endl;
    std::cout << GREEN "Passed: " << summary.passedTests << RESET << std::endl;
    std::cout << RED "Failed: " << summary.failedTests << RESET << std::endl;
    std::cout << YELLOW "Skipped: " << summary.skippedTests << RESET << std::endl;
    std::cout << "Pass Rate: " << std::fixed << std::setprecision(1) << summary.getPassRate() << "%" << std::endl;
    std::cout << "Total Duration: " << formatDuration(summary.totalDuration) << std::endl;
    
    if (summary.allTestsPassed()) {
        std::cout << std::endl << BOLD GREEN "🎉 All tests passed!" RESET << std::endl;
    } else if (summary.failedTests > 0) {
        std::cout << std::endl << BOLD RED "❌ " << summary.failedTests << " test(s) failed!" RESET << std::endl;
    }
}

void TestManager::printDetailedResults(const TestSummary& summary) const {
    std::cout << std::endl << BOLD "Detailed Results:" RESET << std::endl;
    std::cout << "=================" << std::endl;
    
    for (const auto& result : summary.results) {
        std::cout << result.testClass << "::" << result.testName << " - ";
        printColoredStatus(result.status);
        std::cout << " (" << formatDuration(result.duration) << ")";
        
        if (!result.message.empty()) {
            std::cout << std::endl << "  Message: " << result.message;
        }
        std::cout << std::endl;
    }
}

void TestManager::printColoredStatus(TestStatus status) const {
    switch (status) {
        case TestStatus::PASSED:
            std::cout << GREEN "PASSED" RESET;
            break;
        case TestStatus::FAILED:
            std::cout << RED "FAILED" RESET;
            break;
        case TestStatus::SKIPPED:
            std::cout << YELLOW "SKIPPED" RESET;
            break;
    }
}

std::string TestManager::formatDuration(std::chrono::milliseconds duration) const {
    auto ms = duration.count();
    if (ms < 1000) {
        return std::to_string(ms) + "ms";
    } else {
        double seconds = ms / 1000.0;
        return std::to_string(seconds) + "s";
    }
}