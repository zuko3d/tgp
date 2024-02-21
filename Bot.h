#pragma once

#include "Action.h"
#include "GameState.h"
// #include "GameEngine.h"
#include "Types.h"

class IBot {
public:
    virtual Race chooseRace(const GameState& gs, const std::vector<Race>& races) = 0;
    virtual TerrainType chooseTerrainType(const GameState& gs, const std::vector<TerrainType>& colors) = 0;
    virtual int chooseRoundBooster(const GameState& gs) = 0;

    virtual FullAction chooseAction(const GameState& gs, const std::vector<Action>& actions) = 0;

    virtual GodColor chooseGodToMove(const GameState& gs, int amount) = 0;
    virtual FlatMap<BookColor, int8_t, 4> chooseBookColorToGet(const GameState& gs, int amount) = 0;
    virtual FlatMap<BookColor, int8_t, 4> chooseBooksToSpend(const GameState& gs, int amount) = 0;

    // How many bricks you want to spend on terraforming POS hex
    virtual int8_t chooseBricks(const GameState& gs, int8_t pos) = 0;

    virtual bool wannaBuildMine(const GameState& gs, int8_t coord) = 0;
    virtual bool wannaCharge(const GameState& gs, int amount) = 0;
    
    virtual FedTileOrigin chooseFedTile(const GameState& gs) = 0;

    virtual TechTile chooseTechTile(const GameState& gs) = 0;

    virtual int8_t choosePlaceToSpade(const GameState& gs, int amount, const std::vector<int8_t>& possiblePos) = 0;
    virtual int8_t choosePlaceForBridge(const GameState& gs, const std::vector<int8_t>& possiblePos) = 0;
    virtual int8_t choosePlaceToBuildForFree(const GameState& gs, Building building, const std::vector<int8_t>& possiblePos) = 0;

    virtual void triggerFinal(const GameState& gs) { };
};
