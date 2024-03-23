#pragma once

#include "Action.h"
#include "Bot.h"
#include "GameState.h"
#include "Types.h"

#include <functional>
#include <optional>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>

int spadesNeeded(TerrainType src, TerrainType dst);

class GameEngine {
public:
    GameEngine(std::vector<IBot*> bots, bool withLogs = false, bool withStats = false);
    void reset();
    
    void initializeRandomly(GameState& gs, std::default_random_engine& g) const;

    void playGame(GameState& gs) const;

    void doFreeActionMarket(FreeActionMarketType action, GameState& gs) const;
    void doAction(Action action, GameState& gs) const;
    void dealWithUpkeep(GameState& gs) const;
    void doAfterTurnActions(GameState& gs) const;
    void doTurnGuided(GameState& gs) const;
    void advanceGs(GameState& gs) const;
    std::vector<Action> generateActions(const GameState& gs) const;

    void checkFederation(GameState& gs) const;
    void doFinalScoring(GameState& gs) const;

    void pushButton(int8_t buttonIdx, int param, GameState& gs) const;
    void buildBridge(GameState& gs) const;

    void awardWp(int amount, GameState& gs) const;
    void spendResources(IncomableResources resources, GameState& gs) const;
    void spendResources(Resources resources, GameState& gs) const;
    void awardResources(IncomableResources resources, GameState& gs) const;
    void awardResources(Resources resources, GameState& gs) const;
    
    void awardInnovation(Innovation inno, GameState& gs) const;
    void awardTechTile(TechTile tile, GameState& gs) const;
    void awardBooster(int boosterIdx, GameState& gs) const;
    void awardFedTile(FedTileOrigin tile, GameState& gs) const;

    int charge(int amount, GameState& gs) const;

    void putManToGod(GodColor color, bool discard, GameState& gs) const;
    void upgradeNav(GameState& gs, bool forFree = false) const;
    void upgradeTerraform(GameState& gs, bool forFree = false) const;

    void useSpades(int amount, GameState& gs) const;
    void terraformAndBuildMine(int8_t pos, bool build, GameState& gs) const;
    void buildForFree(int8_t pos, Building building, bool isNeutral, GameState& gs) const;
    
    bool gameEnded(const GameState& gs) const;
    // static std::vector<ResizableArray<uint16_t, 6>> generateFieldTopology(int mapSize);

    const std::vector<int8_t>& someHexes(bool onlyInReach, bool onlyNative, const GameState& gs, int cubesDetained = 0, int freeSpades = 0) const;

    void log(const std::string& str) const;

    int moveGod(int amount, GodColor godColor, GameState& gs) const;
    void upgradeBuilding(int8_t pos, Building building, GameState& gs, int palaceIdx = -1) const;
    void terraform(int8_t pos, int amount, GameState& gs) const;
    void buildBridge(int8_t pos, GameState& gs) const;
    void buildMine(int8_t pos, GameState& gs) const;

    void setLogger(std::function<void(const std::string&)> logger);

private:
    int countGroups(GameState& gs) const;

    Race getRace(const GameState& gs) const;
    PlayerState& getPs(GameState& gs) const;
    TerrainType getColor(const GameState& gs) const;

    void chargeOpp(int8_t pos, GameState& gs) const;

    InnoPrice getInnoFullPrice(int pos, const GameState& gs) const;

    std::vector<int8_t> terraformableHexes(const GameState& gs) const;

    std::vector<IBot*> bots_;

    bool withLogs_ = false;
    bool withStats_ = false;

    std::function<void(const std::string&)> logger_;
};
