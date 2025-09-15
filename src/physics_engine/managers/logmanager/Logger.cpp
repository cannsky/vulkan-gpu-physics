#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (logFile && logFile->is_open()) {
        logFile->close();
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    minLogLevel = level;
}

void Logger::enableCategory(LogCategory category, bool enabled) {
    std::lock_guard<std::mutex> lock(logMutex);
    categoryEnabled[static_cast<int>(category)] = enabled;
}

void Logger::disableCategory(LogCategory category) {
    enableCategory(category, false);
}

void Logger::setOutputFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (logFile && logFile->is_open()) {
        logFile->close();
    }
    
    logFile = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (!logFile->is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        logFile.reset();
    }
}

void Logger::enableConsoleOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(logMutex);
    consoleOutput = enabled;
}

void Logger::enableTimestamps(bool enabled) {
    std::lock_guard<std::mutex> lock(logMutex);
    timestampsEnabled = enabled;
}

void Logger::log(LogLevel level, LogCategory category, const std::string& message) {
    if (!shouldLog(level, category)) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string formattedMessage = formatMessage(level, category, message);
    
    if (consoleOutput) {
        std::cout << formattedMessage << std::endl;
    }
    
    if (logFile && logFile->is_open()) {
        *logFile << formattedMessage << std::endl;
        logFile->flush();
    }
}

void Logger::trace(LogCategory category, const std::string& message) {
    log(LogLevel::TRACE, category, message);
}

void Logger::debug(LogCategory category, const std::string& message) {
    log(LogLevel::DEBUG, category, message);
}

void Logger::info(LogCategory category, const std::string& message) {
    log(LogLevel::INFO, category, message);
}

void Logger::warn(LogCategory category, const std::string& message) {
    log(LogLevel::WARN, category, message);
}

void Logger::error(LogCategory category, const std::string& message) {
    log(LogLevel::ERROR, category, message);
}

void Logger::logPhysics(LogLevel level, const std::string& message) {
    log(level, LogCategory::PHYSICS, message);
}

void Logger::logCollision(LogLevel level, const std::string& message) {
    log(level, LogCategory::COLLISION, message);
}

void Logger::logRigidBody(LogLevel level, const std::string& message) {
    log(level, LogCategory::RIGIDBODY, message);
}

void Logger::logParticles(LogLevel level, const std::string& message) {
    log(level, LogCategory::PARTICLES, message);
}

void Logger::logVulkan(LogLevel level, const std::string& message) {
    log(level, LogCategory::VULKAN, message);
}

void Logger::logPerformance(LogLevel level, const std::string& message) {
    log(level, LogCategory::PERFORMANCE, message);
}

void Logger::logFrameTime(float frameTime) {
    std::ostringstream oss;
    oss << "Frame time: " << std::fixed << std::setprecision(3) << frameTime * 1000.0f << "ms";
    logPerformance(LogLevel::DEBUG, oss.str());
}

void Logger::logCollisionCount(uint32_t collisionCount) {
    std::ostringstream oss;
    oss << "Active collisions: " << collisionCount;
    logCollision(LogLevel::DEBUG, oss.str());
}

void Logger::logParticleCount(uint32_t particleCount) {
    std::ostringstream oss;
    oss << "Active particles: " << particleCount;
    logParticles(LogLevel::DEBUG, oss.str());
}

void Logger::logRigidBodyCount(uint32_t rigidBodyCount) {
    std::ostringstream oss;
    oss << "Active rigid bodies: " << rigidBodyCount;
    logRigidBody(LogLevel::DEBUG, oss.str());
}

bool Logger::shouldLog(LogLevel level, LogCategory category) const {
    return (level >= minLogLevel) && categoryEnabled[static_cast<int>(category)];
}

std::string Logger::formatMessage(LogLevel level, LogCategory category, const std::string& message) const {
    std::ostringstream oss;
    
    if (timestampsEnabled) {
        oss << "[" << getCurrentTimestamp() << "] ";
    }
    
    oss << "[" << logLevelToString(level) << "] ";
    oss << "[" << logCategoryToString(category) << "] ";
    oss << message;
    
    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::logCategoryToString(LogCategory category) const {
    switch (category) {
        case LogCategory::GENERAL:     return "GENERAL   ";
        case LogCategory::PHYSICS:     return "PHYSICS   ";
        case LogCategory::COLLISION:   return "COLLISION ";
        case LogCategory::RIGIDBODY:   return "RIGIDBODY ";
        case LogCategory::PARTICLES:   return "PARTICLES ";
        case LogCategory::VULKAN:      return "VULKAN    ";
        case LogCategory::PERFORMANCE: return "PERFORMANCE";
        default: return "UNKNOWN   ";
    }
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}