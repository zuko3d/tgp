#pragma once

#include "ResizableArray.h"
#include "Types.h"

#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

struct GameState;
struct PrecalcCache;

struct FieldOrigin {
    static constexpr size_t FIELD_SIZE = 91;
    static constexpr size_t TOTAL_BRIDGES = 18;

    std::array<TerrainType, FIELD_SIZE> basicType;
    std::array<bool, FIELD_SIZE> onEdge_;
    std::array<bool, FIELD_SIZE> isNearRiver;
    std::array<ResizableArray<int8_t, 6>, FIELD_SIZE> neibs;
    std::array<ResizableArray<int8_t, 4>, FIELD_SIZE> bridgeIds;
    std::array<std::pair<int8_t, int8_t>, TOTAL_BRIDGES> bridgeConnections;
    std::array<std::array<std::vector<int8_t>, FIELD_SIZE>, 5> reachable;
};

// Don't forget to make it Copy-on-write
class Field {
public:
    static Field& newField(PrecalcCache& cache);

    std::vector<int8_t> buildingByPlayer(Building b, int p, bool withNeutrals = false) const;
    bool hasAdjacentEnemies(int8_t pos, int owner) const;
    int adjacentEnemiesPower(int8_t pos, int owner) const;
    int countReachableBuildings(int owner, int reach) const;
    std::vector<int8_t> buildableBridges(int owner) const;
    
    std::array<int8_t, FieldOrigin::FIELD_SIZE> bfs(int owner, int reach) const;

    std::vector<int8_t> reachable(int owner, int range, TerrainType color = TerrainType::None) const;
    ResizableArray<int8_t, 10> adjacent(int pos) const;

    std::array<TerrainType, FieldOrigin::FIELD_SIZE> type;
    std::array<int8_t, FieldOrigin::TOTAL_BRIDGES> bridges; // owner
    std::array<BuildingOnMap, FieldOrigin::FIELD_SIZE> building = {};
    std::array<ResizableArray<int8_t, 22>, 2> ownedByPlayer = {};
    int stateIdx = 0;
    std::array<int8_t, 2> fedsCount = { 0, 0 };

    void populateField(GameState& gs, FieldActionType action, int pos, int param1 = 0, int param2 = 0);
};

struct PrecalcCache {
    std::unordered_map<uint64_t, std::vector<int8_t>> someHexesCache_;
    std::unordered_map<uint64_t, uint64_t> fieldActionsCache_;
    std::vector<Field> fieldByState_;
    std::shared_ptr<std::mutex> populateFieldMutex_ = std::shared_ptr<std::mutex>(new std::mutex);

    void reset() {
        std::lock_guard lock(*populateFieldMutex_);
        someHexesCache_.clear();
        fieldActionsCache_.clear();
        fieldByState_.clear();
    }
};