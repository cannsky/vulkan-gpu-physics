#include "ConcreteEntity.h"
#include "ConcreteComponents.h"
#include "../components.h"
#include <algorithm>

namespace cpu_physics {
namespace concrete {

uint32_t ConcreteEntityFactory::nextEntityId = 1;

// ConcreteEntity implementation
ConcreteEntity::ConcreteEntity(uint32_t id) : entityId(id) {
}

uint32_t ConcreteEntity::getId() const {
    return entityId;
}

bool ConcreteEntity::isActive() const {
    return active;
}

void ConcreteEntity::setActive(bool active) {
    this->active = active;
}

bool ConcreteEntity::addComponent(std::unique_ptr<interfaces::CPUPhysicsComponent> component) {
    if (!component) {
        return false;
    }

    auto componentType = component->getType();
    
    // Check if component already exists
    if (components.find(componentType) != components.end()) {
        return false; // Component already exists
    }

    components[componentType] = std::move(component);
    return true;
}

bool ConcreteEntity::removeComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) {
    auto it = components.find(componentType);
    if (it == components.end()) {
        return false;
    }

    components.erase(it);
    return true;
}

interfaces::CPUPhysicsComponent* ConcreteEntity::getComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) {
    auto it = components.find(componentType);
    if (it == components.end()) {
        return nullptr;
    }
    return it->second.get();
}

const interfaces::CPUPhysicsComponent* ConcreteEntity::getComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) const {
    auto it = components.find(componentType);
    if (it == components.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool ConcreteEntity::hasComponent(interfaces::CPUPhysicsComponent::ComponentType componentType) const {
    return components.find(componentType) != components.end();
}

std::vector<interfaces::CPUPhysicsComponent*> ConcreteEntity::getAllComponents() {
    std::vector<interfaces::CPUPhysicsComponent*> componentList;
    componentList.reserve(components.size());
    
    for (auto& pair : components) {
        componentList.push_back(pair.second.get());
    }
    
    return componentList;
}

std::vector<const interfaces::CPUPhysicsComponent*> ConcreteEntity::getAllComponents() const {
    std::vector<const interfaces::CPUPhysicsComponent*> componentList;
    componentList.reserve(components.size());
    
    for (const auto& pair : components) {
        componentList.push_back(pair.second.get());
    }
    
    return componentList;
}

size_t ConcreteEntity::getComponentCount() const {
    return components.size();
}

bool ConcreteEntity::validate() const {
    // Validate all components
    for (const auto& pair : components) {
        if (!pair.second || !pair.second->validate()) {
            return false;
        }
    }
    
    return true;
}

void ConcreteEntity::reset() {
    components.clear();
    active = true;
    physicsLayer = 0;
    userData = nullptr;
}

std::unique_ptr<interfaces::CPUPhysicsEntity> ConcreteEntity::clone() const {
    auto clonedEntity = std::make_unique<ConcreteEntity>(entityId);
    
    clonedEntity->active = active;
    clonedEntity->physicsLayer = physicsLayer;
    clonedEntity->userData = userData;
    
    // Clone all components
    for (const auto& pair : components) {
        auto clonedComponent = pair.second->clone();
        clonedEntity->components[pair.first] = std::move(clonedComponent);
    }
    
    return std::move(clonedEntity);
}

uint32_t ConcreteEntity::getPhysicsLayer() const {
    return physicsLayer;
}

void ConcreteEntity::setPhysicsLayer(uint32_t layer) {
    physicsLayer = layer;
}

void* ConcreteEntity::getUserData() const {
    return userData;
}

void ConcreteEntity::setUserData(void* userData) {
    this->userData = userData;
}

size_t ConcreteEntity::componentTypeToIndex(interfaces::CPUPhysicsComponent::ComponentType type) const {
    return static_cast<size_t>(type);
}

// ConcreteEntityFactory implementation
std::unique_ptr<ConcreteEntity> ConcreteEntityFactory::createBasicEntity(uint32_t id) {
    return std::make_unique<ConcreteEntity>(id);
}

std::unique_ptr<ConcreteEntity> ConcreteEntityFactory::createRigidBodyEntity(
    uint32_t id,
    float x, float y, float z,
    float width, float height, float depth,
    float mass,
    uint32_t layer) {
    
    auto entity = std::make_unique<ConcreteEntity>(id);
    entity->setPhysicsLayer(layer);
    
    // Create transform component
    TransformComponent transform;
    transform.position[0] = x;
    transform.position[1] = y;
    transform.position[2] = z;
    transform.rotation[0] = 1.0f; // w
    transform.rotation[1] = 0.0f; // x
    transform.rotation[2] = 0.0f; // y
    transform.rotation[3] = 0.0f; // z
    transform.scale[0] = 1.0f;
    transform.scale[1] = 1.0f;
    transform.scale[2] = 1.0f;
    
    auto transformComponent = std::make_unique<ConcreteTransformComponent>(std::move(transform));
    entity->addComponent(std::move(transformComponent));
    
    // Create physics component
    PhysicsComponent physics;
    physics.mass = mass;
    physics.invMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
    physics.isStatic = (mass <= 0.0f);
    physics.useGravity = !physics.isStatic;
    physics.restitution = 0.5f;
    physics.friction = 0.3f;
    
    auto physicsComponent = std::make_unique<ConcretePhysicsComponent>(std::move(physics));
    entity->addComponent(std::move(physicsComponent));
    
    // Create box collider component
    BoxColliderComponent collider;
    collider.width = width;
    collider.height = height;
    collider.depth = depth;
    collider.enabled = true;
    
    auto colliderComponent = std::make_unique<ConcreteBoxColliderComponent>(std::move(collider));
    entity->addComponent(std::move(colliderComponent));
    
    return entity;
}

std::unique_ptr<ConcreteEntity> ConcreteEntityFactory::createStaticEntity(
    uint32_t id,
    float x, float y, float z,
    float width, float height, float depth,
    uint32_t layer) {
    
    // Create a rigidbody with zero mass (static)
    return createRigidBodyEntity(id, x, y, z, width, height, depth, 0.0f, layer);
}

} // namespace concrete
} // namespace cpu_physics