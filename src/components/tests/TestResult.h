#pragma once

#include <string>
#include <chrono>

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
    
    TestResult(const std::string& name, const std::string& className) 
        : testName(name), testClass(className), status(TestStatus::PASSED), duration(0) {}
        
    bool isPassed() const { return status == TestStatus::PASSED; }
    bool isFailed() const { return status == TestStatus::FAILED; }
    bool isSkipped() const { return status == TestStatus::SKIPPED; }
    
    void markPassed(const std::string& msg = "") {
        status = TestStatus::PASSED;
        message = msg;
    }
    
    void markFailed(const std::string& msg) {
        status = TestStatus::FAILED;
        message = msg;
    }
    
    void markSkipped(const std::string& msg = "") {
        status = TestStatus::SKIPPED;
        message = msg;
    }
    
    void setDuration(std::chrono::milliseconds dur) {
        duration = dur;
    }
};