#pragma once

#include "FlatMap.h"
#include "ResizableArray.h"
#include "Types.h"

struct PlayerState {
    int8_t countBuildings(Building building) const {
        constexpr int8_t startingBuildingsAvailable[] = {9, 4, 1, 3, 1};

        if (SC(building) < 5) {
            return startingBuildingsAvailable[SC(building)] - buildingsAvailable[building] + neutralBuildingsAmount[building];
        } else {
            return neutralBuildingsAmount[building];
        }
    }

    Resources resources = Resources{.gold = 15, .cube = 3, .humans = 0, .gods = {0, 0, 0, 0}, .books = {0, 0, 0, 0}, .winPoints = 20};
    std::array<uint8_t, 3> mana = {5, 7, 0};

    FlatMap<EventType, uint8_t, 13> wpPerEvent;
    FlatMap<TechTile, bool, 12> techTiles;
    ResizableArray<FederationTile, 6> feds;
    ResizableArray<Button, 6> buttons;
    ResizableArray<Innovation, 3> innovations;
    FlatMap<Building, int8_t, 5> buildingsAvailable = {9, 4, 1, 3, 1};
    FlatMap<Building, int8_t, 7> neutralBuildingsAmount;
    IncomableResources additionalIncome;
    int8_t fedsCount = 0;
    int8_t palaceIdx = -1;
    int8_t bridgesLeft = 3;
    int8_t humansLeft = 7;
    int8_t navLevel = 0;
    int8_t tfLevel = 0;
    int8_t annexLeft = 0;

    int8_t currentRoundBoosterOriginIdx = -1;
    Button boosterButton;

    bool passed = false;
};
