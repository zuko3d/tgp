#pragma once

#include "Action.h"
#include "GameState.h"
// #include "GameEngine.h"
#include "Types.h"

class IBot {
public:
    Race chooseRace(const GameState& gs, const std::vector<Race>& races);
    TerrainType chooseTerrainType(const GameState& gs, const std::vector<TerrainType>& colors);
    int chooseRoundBooster(const GameState& gs);

    FullAction chooseAction(GameState& gs, const std::vector<Action>& actions);

    GodColor chooseGodToMove(const GameState& gs, int amount);
    std::array<int8_t, 4> chooseBookColorToGet(const GameState& gs, int amount);
    std::array<int8_t, 4> chooseBooksToSpend(const GameState& gs, int amount);

    // How many bricks you want to spend on terraforming POS hex
    int8_t chooseBricks(const GameState& gs, int8_t pos);

    int8_t choosePlaceToSpade(const GameState& gs, int amount);
    int8_t choosePlaceForBridge(const GameState& gs, const std::vector<int8_t>& possiblePos);

    bool wannaBuildMine(const GameState& gs, int8_t coord);
    bool wannaCharge(const GameState& gs, int amount);
    
    FedTileOrigin chooseFedTile(const GameState& gs);

    int8_t chooseTechTile(const GameState& gs);
    int8_t chooseInnovation(const GameState& gs);
    int8_t choosePalace(const GameState& gs);

    int8_t choosePlaceToBuildForFree(const GameState& gs, Building building, bool nativeOnly);
};
