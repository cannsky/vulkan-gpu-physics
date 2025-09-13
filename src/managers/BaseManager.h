#pragma once

/**
 * Base interface for all singleton managers in the physics engine.
 * Provides common lifecycle methods and ensures consistent singleton pattern.
 */
class BaseManager {
public:
    virtual ~BaseManager() = default;
    
    /**
     * Initialize the manager. Should be called after getting the instance.
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;
    
    /**
     * Clean up the manager resources.
     */
    virtual void cleanup() = 0;
    
    /**
     * Check if the manager has been initialized.
     * @return true if initialized, false otherwise
     */
    virtual bool isInitialized() const = 0;

protected:
    BaseManager() = default;
    
    // Non-copyable
    BaseManager(const BaseManager&) = delete;
    BaseManager& operator=(const BaseManager&) = delete;
    
    // Non-movable  
    BaseManager(BaseManager&&) = delete;
    BaseManager& operator=(BaseManager&&) = delete;
};