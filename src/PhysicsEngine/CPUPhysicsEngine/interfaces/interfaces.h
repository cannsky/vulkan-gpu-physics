#pragma once

/**
 * CPU Physics Engine Interfaces
 * 
 * This header provides access to all abstract interfaces for the CPU physics engine.
 * These interfaces define the contracts that concrete implementations must follow
 * for components, entities, and systems.
 * 
 * Usage:
 *   #include "interfaces/interfaces.h"
 * 
 * This will include:
 *   - CPUPhysicsComponent: Abstract base for all physics components
 *   - CPUPhysicsEntity: Abstract base for physics entities
 *   - CPUPhysicsSystem: Abstract base for physics systems
 */

#include "CPUPhysicsComponent.h"
#include "CPUPhysicsEntity.h"
#include "CPUPhysicsSystem.h"

namespace cpu_physics {
namespace interfaces {

/**
 * Version information for the interfaces
 */
struct InterfaceVersion {
    static constexpr uint32_t MAJOR = 1;
    static constexpr uint32_t MINOR = 0;
    static constexpr uint32_t PATCH = 0;
    
    static constexpr const char* VERSION_STRING = "1.0.0";
};

/**
 * Utility functions for working with interfaces
 */
namespace utils {

/**
 * Create a human-readable string from a component type
 * @param type Component type
 * @return String representation
 */
inline const char* componentTypeToString(CPUPhysicsComponent::ComponentType type) {
    switch (type) {
        case CPUPhysicsComponent::ComponentType::TRANSFORM:
            return "Transform";
        case CPUPhysicsComponent::ComponentType::PHYSICS:
            return "Physics";
        case CPUPhysicsComponent::ComponentType::BOX_COLLIDER:
            return "BoxCollider";
        case CPUPhysicsComponent::ComponentType::SPHERE_COLLIDER:
            return "SphereCollider";
        case CPUPhysicsComponent::ComponentType::CUSTOM:
            return "Custom";
        default:
            return "Unknown";
    }
}

/**
 * Create a human-readable string from a system type
 * @param type System type
 * @return String representation
 */
inline const char* systemTypeToString(CPUPhysicsSystem::SystemType type) {
    switch (type) {
        case CPUPhysicsSystem::SystemType::COLLISION:
            return "Collision";
        case CPUPhysicsSystem::SystemType::INTEGRATION:
            return "Integration";
        case CPUPhysicsSystem::SystemType::RENDERING:
            return "Rendering";
        case CPUPhysicsSystem::SystemType::CONSTRAINT:
            return "Constraint";
        case CPUPhysicsSystem::SystemType::PARTICLE:
            return "Particle";
        case CPUPhysicsSystem::SystemType::CUSTOM:
            return "Custom";
        default:
            return "Unknown";
    }
}

/**
 * Create a human-readable string from a system priority
 * @param priority System priority
 * @return String representation
 */
inline const char* systemPriorityToString(CPUPhysicsSystem::Priority priority) {
    switch (priority) {
        case CPUPhysicsSystem::Priority::HIGHEST:
            return "Highest";
        case CPUPhysicsSystem::Priority::HIGH:
            return "High";
        case CPUPhysicsSystem::Priority::NORMAL:
            return "Normal";
        case CPUPhysicsSystem::Priority::LOW:
            return "Low";
        case CPUPhysicsSystem::Priority::LOWEST:
            return "Lowest";
        default:
            return "Unknown";
    }
}

} // namespace utils
} // namespace interfaces
} // namespace cpu_physics