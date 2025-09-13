#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <mutex>

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4
};

enum class LogCategory {
    GENERAL = 0,
    PHYSICS = 1,
    COLLISION = 2,
    RIGIDBODY = 3,
    PARTICLES = 4,
    VULKAN = 5,
    PERFORMANCE = 6
};

class Logger {
public:
    static Logger& getInstance();
    
    // Configuration
    void setLogLevel(LogLevel level);
    void enableCategory(LogCategory category, bool enabled = true);
    void disableCategory(LogCategory category);
    void setOutputFile(const std::string& filename);
    void enableConsoleOutput(bool enabled = true);
    void enableTimestamps(bool enabled = true);
    
    // Logging functions
    void log(LogLevel level, LogCategory category, const std::string& message);
    void trace(LogCategory category, const std::string& message);
    void debug(LogCategory category, const std::string& message);
    void info(LogCategory category, const std::string& message);
    void warn(LogCategory category, const std::string& message);
    void error(LogCategory category, const std::string& message);
    
    // Convenience functions for specific categories
    void logPhysics(LogLevel level, const std::string& message);
    void logCollision(LogLevel level, const std::string& message);
    void logRigidBody(LogLevel level, const std::string& message);
    void logParticles(LogLevel level, const std::string& message);
    void logVulkan(LogLevel level, const std::string& message);
    void logPerformance(LogLevel level, const std::string& message);
    
    // Performance logging
    void logFrameTime(float frameTime);
    void logCollisionCount(uint32_t collisionCount);
    void logParticleCount(uint32_t particleCount);
    void logRigidBodyCount(uint32_t rigidBodyCount);

private:
    Logger() = default;
    ~Logger();
    
    bool shouldLog(LogLevel level, LogCategory category) const;
    std::string formatMessage(LogLevel level, LogCategory category, const std::string& message) const;
    std::string logLevelToString(LogLevel level) const;
    std::string logCategoryToString(LogCategory category) const;
    std::string getCurrentTimestamp() const;
    
    LogLevel minLogLevel = LogLevel::INFO;
    bool categoryEnabled[7] = {true, true, true, true, true, true, true}; // All categories enabled by default
    bool consoleOutput = true;
    bool timestampsEnabled = true;
    
    std::unique_ptr<std::ofstream> logFile;
    mutable std::mutex logMutex;
};

// Convenience macros
#define LOG_TRACE(category, message) Logger::getInstance().trace(category, message)
#define LOG_DEBUG(category, message) Logger::getInstance().debug(category, message)
#define LOG_INFO(category, message) Logger::getInstance().info(category, message)
#define LOG_WARN(category, message) Logger::getInstance().warn(category, message)
#define LOG_ERROR(category, message) Logger::getInstance().error(category, message)

#define LOG_PHYSICS_INFO(message) Logger::getInstance().logPhysics(LogLevel::INFO, message)
#define LOG_COLLISION_INFO(message) Logger::getInstance().logCollision(LogLevel::INFO, message)
#define LOG_RIGIDBODY_INFO(message) Logger::getInstance().logRigidBody(LogLevel::INFO, message)
#define LOG_PARTICLES_INFO(message) Logger::getInstance().logParticles(LogLevel::INFO, message)
#define LOG_VULKAN_INFO(message) Logger::getInstance().logVulkan(LogLevel::INFO, message)
#define LOG_PERFORMANCE_INFO(message) Logger::getInstance().logPerformance(LogLevel::INFO, message)