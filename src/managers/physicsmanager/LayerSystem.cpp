#include "LayerSystem.h"
#include <stdexcept>
#include <cstring>

LayerSystem& LayerSystem::getInstance() {
    static LayerSystem instance;
    return instance;
}

LayerSystem::LayerSystem() : nextLayerId(DEFAULT_LAYER + 1) {
    // Initialize interaction matrix to zeros
    std::memset(layerInteractionMatrix, 0, sizeof(layerInteractionMatrix));
    
    initializeDefaultLayers();
}

void LayerSystem::initializeDefaultLayers() {
    // Create default layer
    layerNameToId["Default"] = DEFAULT_LAYER;
    layerIdToName[DEFAULT_LAYER] = "Default";
    
    // Default layer interacts with itself
    setLayerInteraction(DEFAULT_LAYER, DEFAULT_LAYER, true);
}

LayerSystem::LayerId LayerSystem::createLayer(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Layer name cannot be empty");
    }
    
    // Check if layer already exists
    auto it = layerNameToId.find(name);
    if (it != layerNameToId.end()) {
        return it->second;
    }
    
    // Check if we have room for more layers
    if (nextLayerId >= MAX_LAYERS) {
        throw std::runtime_error("Maximum number of layers reached");
    }
    
    LayerId newLayerId = nextLayerId++;
    layerNameToId[name] = newLayerId;
    layerIdToName[newLayerId] = name;
    
    // By default, new layers interact with themselves and the default layer
    setLayerInteraction(newLayerId, newLayerId, true);
    setLayerInteraction(newLayerId, DEFAULT_LAYER, true);
    setLayerInteraction(DEFAULT_LAYER, newLayerId, true);
    
    return newLayerId;
}

LayerSystem::LayerId LayerSystem::getLayerByName(const std::string& name) const {
    auto it = layerNameToId.find(name);
    return (it != layerNameToId.end()) ? it->second : INVALID_LAYER;
}

std::string LayerSystem::getLayerName(LayerId layerId) const {
    auto it = layerIdToName.find(layerId);
    return (it != layerIdToName.end()) ? it->second : "";
}

bool LayerSystem::isValidLayer(LayerId layerId) const {
    return layerId != INVALID_LAYER && layerId < MAX_LAYERS && 
           layerIdToName.find(layerId) != layerIdToName.end();
}

void LayerSystem::setLayerInteraction(LayerId layerA, LayerId layerB, bool canInteract) {
    if (!isValidLayer(layerA) || !isValidLayer(layerB)) {
        throw std::invalid_argument("Invalid layer ID");
    }
    
    if (canInteract) {
        layerInteractionMatrix[layerA] |= (1ULL << layerB);
        layerInteractionMatrix[layerB] |= (1ULL << layerA);
    } else {
        layerInteractionMatrix[layerA] &= ~(1ULL << layerB);
        layerInteractionMatrix[layerB] &= ~(1ULL << layerA);
    }
}

bool LayerSystem::canLayersInteract(LayerId layerA, LayerId layerB) const {
    if (!isValidLayer(layerA) || !isValidLayer(layerB)) {
        return false;
    }
    
    return (layerInteractionMatrix[layerA] & (1ULL << layerB)) != 0;
}

LayerSystem::LayerMask LayerSystem::createLayerMask(LayerId layerId) const {
    if (!isValidLayer(layerId)) {
        return 0;
    }
    
    return 1ULL << layerId;
}

LayerSystem::LayerMask LayerSystem::combineLayerMasks(LayerMask maskA, LayerMask maskB) const {
    return maskA | maskB;
}

bool LayerSystem::testLayerMask(LayerMask mask, LayerId layerId) const {
    if (!isValidLayer(layerId)) {
        return false;
    }
    
    return (mask & (1ULL << layerId)) != 0;
}

void LayerSystem::reset() {
    nextLayerId = DEFAULT_LAYER + 1;
    layerNameToId.clear();
    layerIdToName.clear();
    std::memset(layerInteractionMatrix, 0, sizeof(layerInteractionMatrix));
    
    initializeDefaultLayers();
}

uint32_t LayerSystem::getLayerCount() const {
    return static_cast<uint32_t>(layerIdToName.size());
}