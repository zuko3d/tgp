#pragma once

#include "ResizableArray.h"
#include "Types.h"
#include <memory>

struct FieldOrigin {
    static constexpr size_t FIELD_SIZE = 89;
    static constexpr size_t TOTAL_BRIDGES = 89;

    std::array<TerrainType, FIELD_SIZE> basicType;
    std::array<bool, FIELD_SIZE> onEdge_;
    std::array<ResizableArray<int8_t, 6>, FIELD_SIZE> neibs;
    std::array<ResizableArray<int8_t, 4>, FIELD_SIZE> bridgeIds;
    std::array<std::pair<int8_t, int8_t>, TOTAL_BRIDGES> bridgeConnections;
    std::array<std::array<ResizableArray<int8_t, 32>, FIELD_SIZE>, 5> reachable;
};

// Don't forget to make it Copy-on-write
struct Field {
    std::vector<int8_t> buildingByPlayer(Building b, int p, bool withNeutrals = false);
    bool hasAdjacentEnemies(int8_t pos, int owner);
    int adjacentEnemiesPower(int8_t pos, int owner);
    std::vector<int8_t> buildableBridges(int owner);
    std::vector<int8_t> reachable(int owner, int range, TerrainType color = TerrainType::None);

    int countReachableBuildings(int owner, int reach);
    std::array<int8_t, Field::FIELD_SIZE> Field::bfs(int owner, int reach);

    static constexpr size_t FIELD_SIZE = 89;
    static constexpr size_t TOTAL_BRIDGES = 89;

    ResizableArray<int8_t, 8> adjacent(int pos);

    std::array<TerrainType, FIELD_SIZE> type;
    std::array<int8_t, TOTAL_BRIDGES> bridges = {{ -1 }}; // owner
    std::array<BuildingOnMap, FIELD_SIZE> building;
    std::array<ResizableArray<int8_t, 20>, 2> ownedByPlayer;
    int stateIdx = 0;
};
