#pragma once

#include "Action.h"
#include "Bot.h"
#include "GameState.h"

#include <random>

class GameEngine {
public:
    void initializeRandomly(GameState& gs, std::vector<IBot*> bots, std::default_random_engine& g);

    void doAction(Action action, GameState& gs);
    void generateActions(GameState& gs);

    void awardWp(int amount, GameState& gs);
    void awardResources(IncomableResources resources, GameState& gs);
    void awardResources(Resources resources, GameState& gs);
    void awardTechTile(int tileIdx, GameState& gs); // TODO
    int charge(int amount, GameState& gs);
    int moveGod(int amount, GodColor godColor, GameState& gs);

    void useSpades(int amount, GameState& gs);
    void build(int8_t pos, Building building, bool isNeutral, GameState& gs); // TODO)

    static std::array<IncomableResources, 7> generateFedTiles();
    static std::array<RoundScoreBonus, 16> generateRoundScoreBonuses();
    static std::array<RoundBoosterOrigin, 10> generateRoundBooosters();
    // static std::array<FieldHex, 13> generateFieldHexes();
    static std::array<RaceStartBonus, 12> generateRaceStartBonus();
    static std::array<Palace, 17> generatePalaces();
    static std::array<BookAction, 6> generateBookActions();
    static std::array<MarketAction, 6> generateMarketActions();
    static std::array<LandTypeBonus, 7> generateLandTypeBonuses();
        
    // static std::vector<ResizableArray<uint16_t, 6>> generateFieldTopology(int mapSize);

private:
    void terraform(int8_t pos, int amount, GameState& gs); // TODO

    std::vector<IBot*> bots_;
    std::vector<ResizableArray<uint16_t, 6>> fieldTopology_;
};
