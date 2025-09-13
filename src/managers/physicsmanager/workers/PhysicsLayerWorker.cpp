#include "PhysicsLayerWorker.h"
#include <stdexcept>

PhysicsLayerWorker::PhysicsLayerWorker() : nextLayerId(DEFAULT_LAYER + 1) {
    // Initialize interaction matrix to allow all interactions by default
    for (uint32_t i = 0; i < MAX_LAYERS; ++i) {
        layerInteractionMatrix[i] = ~0ULL; // All bits set
    }
}

bool PhysicsLayerWorker::initialize() {
    reset();
    initializeDefaultLayers();
    return true;
}

void PhysicsLayerWorker::cleanup() {
    reset();
}

void PhysicsLayerWorker::initializeDefaultLayers() {
    layerNameToId["Default"] = DEFAULT_LAYER;
    layerIdToName[DEFAULT_LAYER] = "Default";
}

PhysicsLayerWorker::LayerId PhysicsLayerWorker::createLayer(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Layer name cannot be empty");
    }
    
    // Check if layer already exists
    auto it = layerNameToId.find(name);
    if (it != layerNameToId.end()) {
        return it->second;
    }
    
    // Check if we've reached the maximum number of layers
    if (nextLayerId >= MAX_LAYERS) {
        throw std::runtime_error("Maximum number of layers reached");
    }
    
    LayerId layerId = nextLayerId++;
    layerNameToId[name] = layerId;
    layerIdToName[layerId] = name;
    
    return layerId;
}

PhysicsLayerWorker::LayerId PhysicsLayerWorker::getLayerByName(const std::string& name) const {
    auto it = layerNameToId.find(name);
    return it != layerNameToId.end() ? it->second : INVALID_LAYER;
}

std::string PhysicsLayerWorker::getLayerName(LayerId layerId) const {
    auto it = layerIdToName.find(layerId);
    return it != layerIdToName.end() ? it->second : "";
}

bool PhysicsLayerWorker::isValidLayer(LayerId layerId) const {
    return layerId != INVALID_LAYER && layerId < MAX_LAYERS && 
           layerIdToName.find(layerId) != layerIdToName.end();
}

void PhysicsLayerWorker::setLayerInteraction(LayerId layerA, LayerId layerB, bool canInteract) {
    if (!isValidLayer(layerA) || !isValidLayer(layerB)) {
        return;
    }
    
    LayerMask maskB = 1ULL << layerB;
    LayerMask maskA = 1ULL << layerA;
    
    if (canInteract) {
        layerInteractionMatrix[layerA] |= maskB;
        layerInteractionMatrix[layerB] |= maskA;
    } else {
        layerInteractionMatrix[layerA] &= ~maskB;
        layerInteractionMatrix[layerB] &= ~maskA;
    }
}

bool PhysicsLayerWorker::canLayersInteract(LayerId layerA, LayerId layerB) const {
    if (!isValidLayer(layerA) || !isValidLayer(layerB)) {
        return false;
    }
    
    LayerMask maskB = 1ULL << layerB;
    return (layerInteractionMatrix[layerA] & maskB) != 0;
}

PhysicsLayerWorker::LayerMask PhysicsLayerWorker::createLayerMask(LayerId layerId) const {
    if (!isValidLayer(layerId)) {
        return 0;
    }
    return 1ULL << layerId;
}

PhysicsLayerWorker::LayerMask PhysicsLayerWorker::combineLayerMasks(LayerMask maskA, LayerMask maskB) const {
    return maskA | maskB;
}

bool PhysicsLayerWorker::testLayerMask(LayerMask mask, LayerId layerId) const {
    if (!isValidLayer(layerId)) {
        return false;
    }
    return (mask & (1ULL << layerId)) != 0;
}

void PhysicsLayerWorker::reset() {
    layerNameToId.clear();
    layerIdToName.clear();
    nextLayerId = DEFAULT_LAYER + 1;
    
    // Reset interaction matrix to allow all interactions
    for (uint32_t i = 0; i < MAX_LAYERS; ++i) {
        layerInteractionMatrix[i] = ~0ULL; // All bits set
    }
}

uint32_t PhysicsLayerWorker::getLayerCount() const {
    return static_cast<uint32_t>(layerNameToId.size());
}