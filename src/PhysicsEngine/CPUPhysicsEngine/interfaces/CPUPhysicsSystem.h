#pragma once

#include "CPUPhysicsEntity.h"
#include "CPUPhysicsComponent.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace cpu_physics {
namespace interfaces {

/**
 * Abstract base class for CPU physics systems
 * 
 * This interface defines the common contract that all CPU physics systems
 * must implement. A system processes entities with specific component
 * combinations and performs operations on them.
 */
class CPUPhysicsSystem {
public:
    /**
     * System type identifier for RTTI and factory systems
     */
    enum class SystemType : uint32_t {
        COLLISION = 0,
        INTEGRATION = 1,
        RENDERING = 2,
        CONSTRAINT = 3,
        PARTICLE = 4,
        CUSTOM = 1000  // Starting point for custom system types
    };

    /**
     * System priority levels for execution order
     */
    enum class Priority : uint32_t {
        HIGHEST = 0,
        HIGH = 100,
        NORMAL = 500,
        LOW = 900,
        LOWEST = 999
    };

    CPUPhysicsSystem() = default;
    virtual ~CPUPhysicsSystem() = default;

    // Delete copy constructor and assignment operator to prevent slicing
    CPUPhysicsSystem(const CPUPhysicsSystem&) = delete;
    CPUPhysicsSystem& operator=(const CPUPhysicsSystem&) = delete;

    // Allow move operations
    CPUPhysicsSystem(CPUPhysicsSystem&&) = default;
    CPUPhysicsSystem& operator=(CPUPhysicsSystem&&) = default;

    /**
     * Get the type of this system
     * @return System type identifier
     */
    virtual SystemType getType() const = 0;

    /**
     * Get human-readable name of the system
     * @return String representation of system name
     */
    virtual const char* getName() const = 0;

    /**
     * Get the priority of this system for execution ordering
     * @return System priority
     */
    virtual Priority getPriority() const = 0;

    /**
     * Initialize the system
     * @return true if initialization was successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * Cleanup the system and release resources
     */
    virtual void cleanup() = 0;

    /**
     * Check if the system is initialized and ready to run
     * @return true if system is initialized, false otherwise
     */
    virtual bool isInitialized() const = 0;

    /**
     * Update the system for one simulation step
     * @param deltaTime Time step in seconds
     */
    virtual void update(float deltaTime) = 0;

    /**
     * Update the system with a specific set of entities
     * @param entities Entities to process
     * @param deltaTime Time step in seconds
     */
    virtual void update(const std::vector<CPUPhysicsEntity*>& entities, float deltaTime) = 0;

    /**
     * Check if this system can process the given entity
     * @param entity Entity to check
     * @return true if system can process the entity, false otherwise
     */
    virtual bool canProcessEntity(const CPUPhysicsEntity* entity) const = 0;

    /**
     * Get the component types required by this system
     * @return Vector of required component types
     */
    virtual std::vector<CPUPhysicsComponent::ComponentType> getRequiredComponents() const = 0;

    /**
     * Get the component types optionally used by this system
     * @return Vector of optional component types
     */
    virtual std::vector<CPUPhysicsComponent::ComponentType> getOptionalComponents() const = 0;

    /**
     * Enable or disable the system
     * @param enabled New enabled state
     */
    virtual void setEnabled(bool enabled) = 0;

    /**
     * Check if the system is enabled
     * @return true if system is enabled, false otherwise
     */
    virtual bool isEnabled() const = 0;

    /**
     * Validate the system's current state
     * @return true if system is in a valid state, false otherwise
     */
    virtual bool validate() const = 0;

    /**
     * Reset system to default state
     */
    virtual void reset() = 0;

    /**
     * Get performance statistics for the last update
     * @return Time taken for last update in milliseconds
     */
    virtual float getLastUpdateTime() const = 0;

    /**
     * Get the number of entities processed in the last update
     * @return Entity count
     */
    virtual size_t getLastEntityCount() const = 0;

    /**
     * Set a callback for entity filtering
     * @param filter Filter function that returns true if entity should be processed
     */
    virtual void setEntityFilter(std::function<bool(const CPUPhysicsEntity*)> filter) = 0;

    /**
     * Clear the entity filter
     */
    virtual void clearEntityFilter() = 0;

    /**
     * Get debug information about the system
     * @return Debug string with system state information
     */
    virtual std::string getDebugInfo() const = 0;

protected:
    /**
     * Process a single entity (called by update methods)
     * @param entity Entity to process
     * @param deltaTime Time step in seconds
     */
    virtual void processEntity(CPUPhysicsEntity* entity, float deltaTime) = 0;

    /**
     * Called before processing entities in an update cycle
     * @param deltaTime Time step in seconds
     */
    virtual void preUpdate(float deltaTime) {}

    /**
     * Called after processing entities in an update cycle
     * @param deltaTime Time step in seconds
     */
    virtual void postUpdate(float deltaTime) {}
};

} // namespace interfaces
} // namespace cpu_physics