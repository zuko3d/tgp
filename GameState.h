#pragma once

#include "Field.h"
#include "Player.h"
#include "ResizableArray.h"
#include "Types.h"

#include <memory>

struct StaticGameState {
    std::array<RoundBoosterOrigin, 5> roundBoosters;
    RoundScoreBonus lastRoundBonus;
    std::array<RoundScoreBonus, 6> bonusByRound;
    std::array<Race, 2> playerRaces;
    std::array<TerrainType, 2> playerColors;
    std::array<BookAction, 3> bookActions;
    std::array<MarketAction, 6> marketActions;
    std::array<std::array<TechTile, 3>, 4> techTiles;
    std::array<Palace, 5> palaces;
};

struct GameState
{
    uint8_t activePlayer;
    uint8_t round = 0;

    std::array<RoundBoosterOnBoard, 3> boosters;
    
    std::array<uint8_t, 2> playersOrder;
    std::array<bool, 2> playerPassed;

    std::array<PlayerState, 2> players;

    std::array<int, 7> fedTilesAvailable;
    std::array<Innovation, 6> innovations;
    ResizableArray<uint8_t, 4> palacesAvailable;

    std::shared_ptr<Field> field;
    std::array<std::array<uint8_t, 3>, 4> humansOnGods;

    std::array<uint8_t, 3> bookActionsAvailable;
    std::array<uint8_t, 6> marketActionsAvailable;

    GamePhase phase = GamePhase::Preparation;

    StaticGameState& staticGs;
};
