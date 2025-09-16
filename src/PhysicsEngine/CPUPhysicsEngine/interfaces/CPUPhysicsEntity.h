#pragma once

#include "CPUPhysicsComponent.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <typeindex>

namespace cpu_physics {
namespace interfaces {

/**
 * Abstract base class for CPU physics entities
 * 
 * This interface defines the common contract that all CPU physics entities
 * must implement. An entity is a container for components and represents
 * a single physics object in the simulation.
 */
class CPUPhysicsEntity {
public:
    CPUPhysicsEntity() = default;
    virtual ~CPUPhysicsEntity() = default;

    // Delete copy constructor and assignment operator to prevent slicing
    CPUPhysicsEntity(const CPUPhysicsEntity&) = delete;
    CPUPhysicsEntity& operator=(const CPUPhysicsEntity&) = delete;

    // Allow move operations
    CPUPhysicsEntity(CPUPhysicsEntity&&) = default;
    CPUPhysicsEntity& operator=(CPUPhysicsEntity&&) = default;

    /**
     * Get the unique identifier of this entity
     * @return Entity ID
     */
    virtual uint32_t getId() const = 0;

    /**
     * Check if this entity is active in the simulation
     * @return true if entity is active, false otherwise
     */
    virtual bool isActive() const = 0;

    /**
     * Activate or deactivate this entity
     * @param active New active state
     */
    virtual void setActive(bool active) = 0;

    /**
     * Add a component to this entity
     * @param component Component to add
     * @return true if component was added successfully, false otherwise
     */
    virtual bool addComponent(std::unique_ptr<CPUPhysicsComponent> component) = 0;

    /**
     * Remove a component from this entity by type
     * @param componentType Type of component to remove
     * @return true if component was removed, false if not found
     */
    virtual bool removeComponent(CPUPhysicsComponent::ComponentType componentType) = 0;

    /**
     * Get a component by type
     * @param componentType Type of component to retrieve
     * @return Pointer to component if found, nullptr otherwise
     */
    virtual CPUPhysicsComponent* getComponent(CPUPhysicsComponent::ComponentType componentType) = 0;

    /**
     * Get a component by type (const version)
     * @param componentType Type of component to retrieve
     * @return Pointer to component if found, nullptr otherwise
     */
    virtual const CPUPhysicsComponent* getComponent(CPUPhysicsComponent::ComponentType componentType) const = 0;

    /**
     * Check if entity has a specific component type
     * @param componentType Type of component to check for
     * @return true if entity has the component, false otherwise
     */
    virtual bool hasComponent(CPUPhysicsComponent::ComponentType componentType) const = 0;

    /**
     * Get all components attached to this entity
     * @return Vector of component pointers
     */
    virtual std::vector<CPUPhysicsComponent*> getAllComponents() = 0;

    /**
     * Get all components attached to this entity (const version)
     * @return Vector of const component pointers
     */
    virtual std::vector<const CPUPhysicsComponent*> getAllComponents() const = 0;

    /**
     * Get the number of components attached to this entity
     * @return Component count
     */
    virtual size_t getComponentCount() const = 0;

    /**
     * Validate the entity and all its components
     * @return true if entity is in a valid state, false otherwise
     */
    virtual bool validate() const = 0;

    /**
     * Reset entity to default state (removes all components)
     */
    virtual void reset() = 0;

    /**
     * Clone this entity (deep copy)
     * @return New instance that is a copy of this entity
     */
    virtual std::unique_ptr<CPUPhysicsEntity> clone() const = 0;

    /**
     * Get the physics layer this entity belongs to
     * @return Physics layer ID
     */
    virtual uint32_t getPhysicsLayer() const = 0;

    /**
     * Set the physics layer this entity belongs to
     * @param layer Physics layer ID
     */
    virtual void setPhysicsLayer(uint32_t layer) = 0;

    /**
     * Get user data associated with this entity
     * @return Pointer to user data, or nullptr if none
     */
    virtual void* getUserData() const = 0;

    /**
     * Set user data associated with this entity
     * @param userData Pointer to user data
     */
    virtual void setUserData(void* userData) = 0;
};

} // namespace interfaces
} // namespace cpu_physics