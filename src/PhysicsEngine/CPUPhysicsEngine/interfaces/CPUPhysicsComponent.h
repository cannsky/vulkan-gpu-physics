#pragma once

#include <cstdint>
#include <memory>

namespace cpu_physics {
namespace interfaces {

/**
 * Abstract base class for CPU physics components
 * 
 * This interface defines the common contract that all CPU physics components
 * must implement. It provides a foundation for polymorphic behavior and
 * allows for different types of physics components to be handled uniformly.
 */
class CPUPhysicsComponent {
public:
    /**
     * Component type identifier for RTTI and factory systems
     */
    enum class ComponentType : uint32_t {
        TRANSFORM = 0,
        PHYSICS = 1,
        BOX_COLLIDER = 2,
        SPHERE_COLLIDER = 3,
        CUSTOM = 1000  // Starting point for custom component types
    };

    CPUPhysicsComponent() = default;
    virtual ~CPUPhysicsComponent() = default;

    // Delete copy constructor and assignment operator to prevent slicing
    CPUPhysicsComponent(const CPUPhysicsComponent&) = delete;
    CPUPhysicsComponent& operator=(const CPUPhysicsComponent&) = delete;

    // Allow move operations
    CPUPhysicsComponent(CPUPhysicsComponent&&) = default;
    CPUPhysicsComponent& operator=(CPUPhysicsComponent&&) = default;

    /**
     * Get the type of this component
     * @return Component type identifier
     */
    virtual ComponentType getType() const = 0;

    /**
     * Get human-readable name of the component type
     * @return String representation of component type
     */
    virtual const char* getTypeName() const = 0;

    /**
     * Check if this component is enabled/active
     * @return true if component is enabled, false otherwise
     */
    virtual bool isEnabled() const = 0;

    /**
     * Enable or disable this component
     * @param enabled New enabled state
     */
    virtual void setEnabled(bool enabled) = 0;

    /**
     * Validate the component's current state
     * @return true if component is in a valid state, false otherwise
     */
    virtual bool validate() const = 0;

    /**
     * Reset component to default state
     */
    virtual void reset() = 0;

    /**
     * Clone this component (deep copy)
     * @return New instance that is a copy of this component
     */
    virtual std::unique_ptr<CPUPhysicsComponent> clone() const = 0;

    /**
     * Get the size in bytes of this component's data
     * @return Size in bytes
     */
    virtual size_t getDataSize() const = 0;
};

} // namespace interfaces
} // namespace cpu_physics