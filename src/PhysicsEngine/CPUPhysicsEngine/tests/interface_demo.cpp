#include "../interfaces/interfaces.h"
#include "../concrete/ConcreteEntity.h"
#include "../concrete/ConcreteComponents.h"
#include "../systems/EnhancedCPUPhysicsCollisionSystem.h"
#include "../managers/ECSManager/ECSManager.h"
#include "../../managers/logmanager/Logger.h"
#include <iostream>
#include <memory>

/**
 * Test program demonstrating the new CPU Physics Engine interfaces
 */
int main() {
    LOG_INFO(LogCategory::GENERAL, "=== CPU Physics Engine Interface Demo ===");
    
    // Test 1: Create concrete components
    LOG_INFO(LogCategory::GENERAL, "Test 1: Creating concrete components...");
    
    // Create a transform component
    cpu_physics::TransformComponent transform;
    transform.position[0] = 1.0f;
    transform.position[1] = 2.0f;
    transform.position[2] = 3.0f;
    
    auto concreteTransform = std::make_unique<cpu_physics::concrete::ConcreteTransformComponent>(transform);
    LOG_INFO(LogCategory::GENERAL, "Created " + std::string(concreteTransform->getTypeName()));
    
    // Create a physics component
    cpu_physics::PhysicsComponent physics;
    physics.mass = 2.5f;
    physics.invMass = 1.0f / 2.5f;
    
    auto concretePhysics = std::make_unique<cpu_physics::concrete::ConcretePhysicsComponent>(physics);
    LOG_INFO(LogCategory::GENERAL, "Created " + std::string(concretePhysics->getTypeName()));
    
    // Create a box collider component
    cpu_physics::BoxColliderComponent collider;
    collider.width = 2.0f;
    collider.height = 2.0f;
    collider.depth = 2.0f;
    
    auto concreteCollider = std::make_unique<cpu_physics::concrete::ConcreteBoxColliderComponent>(collider);
    LOG_INFO(LogCategory::GENERAL, "Created " + std::string(concreteCollider->getTypeName()));
    
    // Test component validation
    bool allValid = concreteTransform->validate() && 
                   concretePhysics->validate() && 
                   concreteCollider->validate();
    LOG_INFO(LogCategory::GENERAL, "All components valid: " + std::string(allValid ? "true" : "false"));
    
    // Test 2: Create concrete entity
    LOG_INFO(LogCategory::GENERAL, "\nTest 2: Creating concrete entity...");
    
    auto entity = cpu_physics::concrete::ConcreteEntityFactory::createRigidBodyEntity(
        1, // id
        5.0f, 10.0f, 15.0f, // position
        1.5f, 1.5f, 1.5f,   // dimensions
        3.0f,               // mass
        1                   // layer
    );
    
    LOG_INFO(LogCategory::GENERAL, "Created entity with ID " + std::to_string(entity->getId()));
    LOG_INFO(LogCategory::GENERAL, "Entity has " + std::to_string(entity->getComponentCount()) + " components");
    LOG_INFO(LogCategory::GENERAL, "Entity is active: " + std::string(entity->isActive() ? "true" : "false"));
    LOG_INFO(LogCategory::GENERAL, "Entity physics layer: " + std::to_string(entity->getPhysicsLayer()));
    
    // Check which components the entity has
    bool hasTransform = entity->hasComponent(cpu_physics::interfaces::CPUPhysicsComponent::ComponentType::TRANSFORM);
    bool hasPhysics = entity->hasComponent(cpu_physics::interfaces::CPUPhysicsComponent::ComponentType::PHYSICS);
    bool hasCollider = entity->hasComponent(cpu_physics::interfaces::CPUPhysicsComponent::ComponentType::BOX_COLLIDER);
    
    LOG_INFO(LogCategory::GENERAL, "Has Transform: " + std::string(hasTransform ? "true" : "false"));
    LOG_INFO(LogCategory::GENERAL, "Has Physics: " + std::string(hasPhysics ? "true" : "false"));
    LOG_INFO(LogCategory::GENERAL, "Has BoxCollider: " + std::string(hasCollider ? "true" : "false"));
    
    // Test 3: Enhanced collision system with interfaces
    LOG_INFO(LogCategory::GENERAL, "\nTest 3: Testing enhanced collision system...");
    
    auto ecsManager = std::make_shared<cpu_physics::ECSManager>();
    auto enhancedSystem = std::make_unique<cpu_physics::EnhancedCPUPhysicsCollisionSystem>(ecsManager);
    
    bool systemInit = enhancedSystem->initialize();
    LOG_INFO(LogCategory::GENERAL, "Enhanced system initialized: " + std::string(systemInit ? "true" : "false"));
    
    LOG_INFO(LogCategory::GENERAL, "System type: " + 
        std::string(cpu_physics::interfaces::utils::systemTypeToString(enhancedSystem->getType())));
    LOG_INFO(LogCategory::GENERAL, "System name: " + std::string(enhancedSystem->getName()));
    LOG_INFO(LogCategory::GENERAL, "System priority: " + 
        std::string(cpu_physics::interfaces::utils::systemPriorityToString(enhancedSystem->getPriority())));
    
    // Get required components
    auto requiredComponents = enhancedSystem->getRequiredComponents();
    LOG_INFO(LogCategory::GENERAL, "Required components: " + std::to_string(requiredComponents.size()));
    for (auto componentType : requiredComponents) {
        LOG_INFO(LogCategory::GENERAL, "  - " + 
            std::string(cpu_physics::interfaces::utils::componentTypeToString(componentType)));
    }
    
    // Test 4: Interface version information
    LOG_INFO(LogCategory::GENERAL, "\nTest 4: Interface version information...");
    LOG_INFO(LogCategory::GENERAL, "Interface version: " + 
        std::string(cpu_physics::interfaces::InterfaceVersion::VERSION_STRING));
    LOG_INFO(LogCategory::GENERAL, "Major: " + std::to_string(cpu_physics::interfaces::InterfaceVersion::MAJOR));
    LOG_INFO(LogCategory::GENERAL, "Minor: " + std::to_string(cpu_physics::interfaces::InterfaceVersion::MINOR));
    LOG_INFO(LogCategory::GENERAL, "Patch: " + std::to_string(cpu_physics::interfaces::InterfaceVersion::PATCH));
    
    // Test 5: Entity cloning
    LOG_INFO(LogCategory::GENERAL, "\nTest 5: Testing entity cloning...");
    
    auto clonedEntity = entity->clone();
    LOG_INFO(LogCategory::GENERAL, "Cloned entity ID: " + std::to_string(clonedEntity->getId()));
    LOG_INFO(LogCategory::GENERAL, "Cloned entity components: " + std::to_string(clonedEntity->getComponentCount()));
    LOG_INFO(LogCategory::GENERAL, "Original and clone have same component count: " + 
        std::string(entity->getComponentCount() == clonedEntity->getComponentCount() ? "true" : "false"));
    
    // Cleanup
    enhancedSystem->cleanup();
    
    LOG_INFO(LogCategory::GENERAL, "\n=== All interface tests completed successfully! ===");
    
    return 0;
}