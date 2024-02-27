#pragma once

#include "Action.h"
#include "Bot.h"
#include "GameState.h"
#include "Types.h"

#include <random>

class GameEngine {
public:
    GameEngine(std::vector<IBot*> bots);
    
    void initializeRandomly(GameState& gs, std::default_random_engine& g);

    void playGame(GameState& gs);

    void doFreeActionMarket(FreeActionMarketType action, GameState& gs);
    void doAction(Action action, GameState& gs);
    void doTurnGuided(GameState& gs);
    void advanceGs(GameState& gs);
    std::vector<Action> generateActions(GameState& gs);

    void checkFederation(int8_t pos, bool isBridge, GameState& gs);
    void doFinalScoring(GameState& gs);

    void pushButton(int8_t buttonIdx, int param, GameState& gs);
    void buildBridge(GameState& gs);

    void awardWp(int amount, GameState& gs);
    void spendResources(IncomableResources resources, GameState& gs);
    void spendResources(Resources resources, GameState& gs);
    void awardResources(IncomableResources resources, GameState& gs);
    void awardResources(Resources resources, GameState& gs);
    
    void awardInnovation(Innovation inno, GameState& gs);
    void awardTechTile(TechTile tile, GameState& gs);
    void awardBooster(int boosterIdx, GameState& gs);
    void awardFedTile(FedTileOrigin tile, GameState& gs);

    int charge(int amount, GameState& gs);

    void putManToGod(GodColor color, bool discard, GameState& gs);
    void upgradeNav(GameState& gs, bool forFree = false);
    void upgradeTerraform(GameState& gs, bool forFree = false);

    void useSpades(int amount, GameState& gs);
    void terraformAndBuildMine(int8_t pos, bool build, GameState& gs);
    void buildForFree(int8_t pos, Building building, bool isNeutral, GameState& gs);
    
    bool gameEnded(const GameState& gs) const;
    // static std::vector<ResizableArray<uint16_t, 6>> generateFieldTopology(int mapSize);

private:
    int moveGod(int amount, GodColor godColor, GameState& gs);

    int countGroups(GameState& gs) const;

    Race getRace(const GameState& gs) const;
    PlayerState& getPs(GameState& gs);
    TerrainType getColor(const GameState& gs) const;

    void upgradeBuilding(int8_t pos, Building building, GameState& gs, int palaceIdx = -1);
    void populateField(GameState& gs);

    void chargeOpp(int8_t pos, GameState& gs);

    void buildBridge(int8_t pos, GameState& gs);
    void terraform(int8_t pos, int amount, GameState& gs);
    void buildMine(int8_t pos, GameState& gs);

    InnoPrice getInnoFullPrice(int pos, GameState& gs);

    std::vector<int8_t> someHexes(bool onlyInReach, bool onlyNative, const GameState& gs, int cubesDetained = 0, int freeSpades = 0) const;
    std::vector<int8_t> terraformableHexes(const GameState& gs) const;

    std::vector<IBot*> bots_;
    int fieldStateIdx = 0;
    bool withLogs_ = false;
    bool withStats_ = false;
};
