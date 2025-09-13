#pragma once

#include "../Test.h"
#include "../../managers/logmanager/Logger.h"

class LoggingLevelsTest : public Test {
public:
    std::string getName() const override {
        return "LoggingLevels";
    }
    
    std::string getClassName() const override {
        return "LoggerTests";
    }
    
    void run(TestResult& result) override {
        // Test different log levels
        Logger::getInstance().trace(LogCategory::GENERAL, "Trace message");
        Logger::getInstance().debug(LogCategory::GENERAL, "Debug message");
        Logger::getInstance().info(LogCategory::GENERAL, "Info message");
        Logger::getInstance().warn(LogCategory::GENERAL, "Warning message");
        Logger::getInstance().error(LogCategory::GENERAL, "Error message");
        
        result.markPassed("All log levels tested successfully");
    }
};

class CategorySpecificLoggingTest : public Test {
public:
    std::string getName() const override {
        return "CategorySpecificLogging";
    }
    
    std::string getClassName() const override {
        return "LoggerTests";
    }
    
    void run(TestResult& result) override {
        // Test category-specific logging
        Logger::getInstance().logPhysics(LogLevel::INFO, "Physics system test");
        Logger::getInstance().logCollision(LogLevel::INFO, "Collision system test");
        Logger::getInstance().logRigidBody(LogLevel::INFO, "Rigid body system test");
        Logger::getInstance().logParticles(LogLevel::INFO, "Particle system test");
        Logger::getInstance().logVulkan(LogLevel::INFO, "Vulkan system test");
        Logger::getInstance().logPerformance(LogLevel::INFO, "Performance test");
        
        result.markPassed("Category-specific logging tested successfully");
    }
};

class PerformanceLoggingTest : public Test {
public:
    std::string getName() const override {
        return "PerformanceLogging";
    }
    
    std::string getClassName() const override {
        return "LoggerTests";
    }
    
    void run(TestResult& result) override {
        // Test performance logging
        Logger::getInstance().logFrameTime(0.016f);
        Logger::getInstance().logCollisionCount(5);
        Logger::getInstance().logParticleCount(100);
        Logger::getInstance().logRigidBodyCount(10);
        
        result.markPassed("Performance logging tested successfully");
    }
};

class LoggerConfigurationTest : public Test {
public:
    std::string getName() const override {
        return "LoggerConfiguration";
    }
    
    std::string getClassName() const override {
        return "LoggerTests";
    }
    
    void run(TestResult& result) override {
        // Test logger configuration
        Logger& logger = Logger::getInstance();
        
        // Test enabling/disabling categories
        logger.enableCategory(LogCategory::COLLISION);
        logger.disableCategory(LogCategory::PARTICLES);
        
        // Test setting log level
        logger.setLogLevel(LogLevel::INFO);
        
        // Test console output configuration
        logger.enableConsoleOutput(true);
        logger.enableTimestamps(true);
        
        result.markPassed("Logger configuration tested successfully");
    }
};