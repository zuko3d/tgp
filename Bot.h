#pragma once

#include "Action.h"
#include "GameState.h"
// #include "GameEngine.h"
#include "Types.h"

class IBot {
public:
    Race chooseRace(const GameState& gs);
    int chooseRoundBooster(const GameState& gs);

    void chooseAction(GameState& gs, const std::vector<Action>& actions);

    GodColor chooseGodToMove(const GameState& gs, int amount);
    BookColor chooseBookColor(const GameState& gs, int amount);

    bool wannaBuildMine(const GameState& gs, int8_t coord);

    int8_t choosePlaceToSpade(const GameState& gs, int amount);
    int8_t chooseBricks(const GameState& gs, int8_t pos);

    Race chooseRace(const GameState& gs, std::vector<Race> races);
    TerrainType chooseTerrainType(const GameState& gs, std::vector<TerrainType> colors);

    int8_t choosePlaceToBuildForFree(const GameState& gs, Building building, bool nativeOnly);

    int8_t chooseTechTile(const GameState& gs);
};
