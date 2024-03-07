#pragma once

#include "Field.h"
#include "FlatMap.h"
#include "Player.h"
#include "ResizableArray.h"
#include "Types.h"

#include <memory>

struct StaticGameState {
    // std::array<int, 5> roundBoosters; // origin idx
    int lastRoundBonus;
    std::array<int, 6> bonusByRound; // origin idx
    std::array<Race, 2> playerRaces = { (Race) -1, (Race) -1};
    std::array<TerrainType, 2> playerColors = { (TerrainType) -1, (TerrainType) -1};
    std::array<std::array<TechTile, 3>, 4> techTiles;
    FlatMap<TechTile, Resources, 12> bookAndGodPerTech;
    std::array<Palace, 5> palaces;
    FlatMap<GodColor, int8_t, 4> neutralGods;
};

struct GameState
{
    const Field& field() const { return Field::fieldByState_[fieldIdx]; }

    uint8_t activePlayer;
    uint8_t round = 0;

    ResizableArray<RoundBoosterOnBoard, 5> boosters;
    
    ResizableArray<uint8_t, 2> playersOrder;

    std::array<PlayerState, 2> players;

    FlatMap<FedTileOrigin, uint8_t, 7> fedTilesAvailable;
    std::array<Innovation, 6> innovations;
    ResizableArray<uint8_t, 4> palacesAvailable;

    int fieldIdx;
    FlatMap<GodColor, ResizableArray<uint8_t, 3>, 4> humansOnGods;

    std::array<BookButton, 3> bookActions;
    std::array<MarketButton, 6> marketActions;

    GamePhase phase = GamePhase::Upkeep;

    StaticGameState& staticGs;
};
