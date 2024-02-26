#pragma once

#include "ResizableArray.h"
#include "Types.h"

#include <array>
#include <memory>
#include <vector>

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
struct Field {
    std::vector<int8_t> buildingByPlayer(Building b, int p, bool withNeutrals = false) const;
    bool hasAdjacentEnemies(int8_t pos, int owner) const;
    int adjacentEnemiesPower(int8_t pos, int owner) const;
    int countReachableBuildings(int owner, int reach) const;
    std::vector<int8_t> buildableBridges(int owner) const;
    
    std::array<int8_t, FieldOrigin::FIELD_SIZE> bfs(int owner, int reach) const;

    std::vector<int8_t> reachable(int owner, int range, TerrainType color = TerrainType::None) const;
    ResizableArray<int8_t, 8> adjacent(int pos) const;

    std::array<TerrainType, FieldOrigin::FIELD_SIZE> type;
    std::array<int8_t, FieldOrigin::TOTAL_BRIDGES> bridges; // owner
    std::array<BuildingOnMap, FieldOrigin::FIELD_SIZE> building;
    std::array<ResizableArray<int8_t, 20>, 2> ownedByPlayer;
    int stateIdx = 0;
};
