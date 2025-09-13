#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

/**
 * Physics layer worker for collision filtering.
 * Allows rigidbodies to be assigned to layers and control which layers interact.
 */
class PhysicsLayerWorker {
public:
    using LayerId = uint32_t;
    using LayerMask = uint64_t;
    
    static constexpr LayerId INVALID_LAYER = 0;
    static constexpr LayerId DEFAULT_LAYER = 1;
    static constexpr LayerId MAX_LAYERS = 64;
    
    PhysicsLayerWorker();
    ~PhysicsLayerWorker() = default;
    
    // Initialize the worker
    bool initialize();
    void cleanup();
    
    // Layer management
    LayerId createLayer(const std::string& name);
    LayerId getLayerByName(const std::string& name) const;
    std::string getLayerName(LayerId layerId) const;
    bool isValidLayer(LayerId layerId) const;
    
    // Layer interaction control
    void setLayerInteraction(LayerId layerA, LayerId layerB, bool canInteract);
    bool canLayersInteract(LayerId layerA, LayerId layerB) const;
    
    // Layer mask operations
    LayerMask createLayerMask(LayerId layerId) const;
    LayerMask combineLayerMasks(LayerMask maskA, LayerMask maskB) const;
    bool testLayerMask(LayerMask mask, LayerId layerId) const;
    
    // Utility
    void reset();
    uint32_t getLayerCount() const;

private:
    LayerId nextLayerId;
    std::unordered_map<std::string, LayerId> layerNameToId;
    std::unordered_map<LayerId, std::string> layerIdToName;
    
    // Interaction matrix - layerInteractionMatrix[layerA] & (1 << layerB) determines if they interact
    LayerMask layerInteractionMatrix[MAX_LAYERS];
    
    void initializeDefaultLayers();
};