#pragma once

#include "Field.h"
#include "FlatMap.h"
#include "Player.h"
#include "ResizableArray.h"
#include "Types.h"

#include <memory>

struct StaticGameState {
    FieldOrigin fieldOrigin;
    std::array<RoundBoosterOrigin, 5> roundBoosters;
    RoundScoreBonus lastRoundBonus;
    std::array<RoundScoreBonus, 6> bonusByRound;
    std::array<Race, 2> playerRaces;
    std::array<TerrainType, 2> playerColors;
    std::array<std::array<TechTile, 3>, 4> techTiles;
    FlatMap<TechTile, Resources, 12> bookAndGodPerTech;
    std::array<Palace, 5> palaces;
    FlatMap<GodColor, int8_t, 4> neutralGods;
};

struct GameState
{
    uint8_t activePlayer;
    uint8_t round = 0;

    ResizableArray<RoundBoosterOnBoard, 5> boosters;
    
    std::array<uint8_t, 2> playersOrder;

    std::array<PlayerState, 2> players;

    FlatMap<FedTileOrigin, uint8_t, 7> fedTilesAvailable;
    std::array<Innovation, 6> innovations;
    ResizableArray<uint8_t, 4> palacesAvailable;

    std::shared_ptr<Field> field;
    FlatMap<GodColor, ResizableArray<uint8_t, 3>, 4> humansOnGods;

    std::array<BookButton, 3> bookActions;
    std::array<MarketButton, 6> marketActions;

    GamePhase phase = GamePhase::Preparation;

    StaticGameState& staticGs;
};
