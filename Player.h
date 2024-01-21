#pragma once

#include "ResizableArray.h"
#include "Types.h"

struct PlayerState {
    Resources resources = Resources{.gold = 15, .cube = 3, .humans = 0, .gods = {0, 0, 0, 0}, .books = {0, 0, 0, 0}, .winPoints = 20};
    std::array<uint8_t, 3> mana = {5, 7, 0};

    ResizableArray<TechTile, 8> techTiles;
    ResizableArray<FederationTile, 6> feds;
    uint8_t currentRoundBooster = -1;
    std::array<int8_t, 5> buildingsAvailable = {9, 4, 1, 3, 1};
};
