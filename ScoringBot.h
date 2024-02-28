#pragma once

#include "Bot.h"
#include "GameEngine.h"
#include "Serialize.h"

#include <array>
#include <iostream>
#include <random>

struct ScoreWeights {
    double gold = 0;
    double cube = 0;
    double humans = 0;
    double totalBooks = 0;
    double totalGods = 0;
    double winPoints = 0;

    double goldIncome = 0;
    double cubeIncome = 0;
    double humansIncome = 0;
    double godsIncome = 0;
    double booksIncome = 0;
    double winPointsIncome = 0;
    double manaIncome = 0;

    double targetGod = 0;

    double totalPower = 0;
    double scorePerBuilding[7] = {0};

    double navLevel[4] = {0};
    double tfLevel[3] = {0};

    double reachableHexes[4] = {0}; // per terraforms, 0 = native
};

using AllScoreWeights = std::array<ScoreWeights, 7>; // per round, at round's start

class ScoringBot: public IBot {
public:
    ScoringBot(AllScoreWeights allScoreWeights)
        : allScoreWeights_(allScoreWeights)
        , ownGe_({ this, this })
    { }

    Race chooseRace(const GameState& gs, const std::vector<Race>& races) {
        return races[rng() % races.size()];
    }

    TerrainType chooseTerrainType(const GameState& gs, const std::vector<TerrainType>& colors) {
        return colors[rng() % colors.size()];
    }
    int chooseRoundBooster(const GameState& gs) {
        return rng() % gs.boosters.size();
    }

    double playOut(GameState& gs, int pIdx) {
        // const int nextRound = gs.round + 1;
        while (!ownGe_.gameEnded(gs)) {
            ownGe_.advanceGs(gs);
        }
        return evalPs(gs, pIdx);
    }

    FullAction chooseAction(const GameState& gs, const std::vector<Action>& actions) {
        const auto& ps = gs.players[gs.activePlayer];

        if (ps.mana[1] >= 2 && sum(ps.mana) > 7) {
            return FullAction{
                .preAction = { FreeActionMarketType::BurnMana },
                .action = Action{
                    .type = ActionType::None
                },
                .postAction = {}
            };
        }

        if (greedy_) return chooseActionGreedy(gs, actions);

        auto bestAction = actions.front();
        double bestPts = -1e9;
        for (const auto& action : actions) {
            auto newGs = gs;
            ownGe_.doAction(action, newGs);
            greedy_ = true;
            const auto pts = playOut(newGs, gs.activePlayer);
            greedy_ = false;

            if (!greedy_) std::cerr << "Action " << toJson(action).dump() << " \t pts: " << pts << std::endl;

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = action;
            }
        }

        return FullAction{
            .preAction = {},
            .action = bestAction,
            .postAction = {}
        };
    }

    GodColor chooseGodToMove(const GameState& gs, int amount) {
        return (GodColor) (rng() % 4);
    }

    FlatMap<BookColor, int8_t, 4> chooseBookColorToGet(const GameState& gs, int amount) { 
        FlatMap<BookColor, int8_t, 4> ret = { 0, 0, 0, 0 };
        ret[(BookColor) (rng() % 4)] = amount;
        return ret;
    }
    FlatMap<BookColor, int8_t, 4> chooseBooksToSpend(const GameState& gs, int amount) {
        FlatMap<BookColor, int8_t, 4> ret = { 0, 0, 0, 0 };
        const auto& ps = gs.players[gs.activePlayer];
        assert(sum(ps.resources.books.values()) >= amount);

        for (const auto [color, val]: ps.resources.books) {
            const auto amnt = std::min((int) val, amount);
            amount -= amnt;
            ret[color] = amnt;
            if (amount == 0) break;
        }

        return ret;
    }

    // How many bricks you want to spend on terraforming POS hex
    int8_t chooseBricks(const GameState& gs, int8_t pos) { return 0; }
    
    bool wannaBuildMine(const GameState& gs, int8_t coord) { return true; }
    bool wannaCharge(const GameState& gs, int amount) { return true; }
    
    FedTileOrigin chooseFedTile(const GameState& gs) {
        for (const auto [tile, amnt]: gs.fedTilesAvailable) {
            if (amnt > 0) {
                return tile;
            }
        }
        assert(false);
        return -1;
    }

    TechTile chooseTechTile(const GameState& gs) {
        const auto& ps = gs.players[gs.activePlayer];
        for (int i = 0; i < 12; i++) {
            TechTile tile = (TechTile) i;
            if (!ps.techTiles[tile]) return tile;
        }
        assert(false);
        return (TechTile) -1;
    }

    int8_t choosePlaceToSpade(const GameState& gs, int amount, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        
        return possiblePos[rng() % possiblePos.size()];
    }

    int8_t choosePlaceForBridge(const GameState& gs, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos[rng() % possiblePos.size()];
    }

    int8_t choosePlaceToBuildForFree(const GameState& gs,  Building building, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos[rng() % possiblePos.size()];
    }

private:
    FullAction chooseActionGreedy(const GameState& gs, const std::vector<Action>& actions) {
        auto bestAction = actions.front();
        double bestPts = -1e9;

        for (const auto& action : actions) {
            auto newGs = gs;
            ownGe_.doAction(action, newGs);
            const auto pts = evalPs(newGs, gs.activePlayer);
            if (bestPts < pts) {
                bestPts = pts;
                bestAction = action;
            }
        }

        return FullAction{
            .preAction = {},
            .action = bestAction,
            .postAction = {}
        };
    }

    double evalPs(const GameState& gs, int pIdx);

    std::default_random_engine rng{43};
    AllScoreWeights allScoreWeights_;

    GameEngine ownGe_;
    bool greedy_ = false;
};
