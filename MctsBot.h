#pragma once

#include "Bot.h"
#include "GreedyBot.h"
#include "GameEngine.h"
#include "Serialize.h"

#include <array>
#include <iostream>
#include <memory>
#include <random>

struct MctsNode;

class MctsBot: public IBot {
public:
    MctsBot(IBot* botPtr, AllScoreWeights allScoreWeights, int steps, int roundsDepth)
        : steps_(steps)
        , roundsDepth_(roundsDepth)
        , allScoreWeights_(allScoreWeights)
        , greedyBot_(botPtr)
        , ownGe_({ greedyBot_.get(), greedyBot_.get() })
    { }

    Race chooseRace(const GameState& gs, const std::vector<Race>& races) {
        return races[0];
    }

    TerrainType chooseTerrainType(const GameState& gs, const std::vector<TerrainType>& colors) {
        return colors[0];
    }
    int chooseRoundBooster(const GameState& gs) {
        int bestAction = 0;
        double bestPts = -1e9;
        for (const auto& [action, _] : enumerate(gs.boosters)) {
            auto newGs = gs;
            ownGe_.awardBooster(action, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = action;
            }
        }

        return bestAction;
    }

    double playOut(GameState& gs, int pIdx) {
        // const int nextRound = gs.round + 1;
        while (!ownGe_.gameEnded(gs)) {
            ownGe_.advanceGs(gs);
        }
        // return evalPs(gs, pIdx);
        return gs.players[pIdx].resources.winPoints;
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

        const auto bestAction = buildMcTree(gs);

        return FullAction{
            .preAction = {},
            .action = bestAction,
            .postAction = {}
        };
    }

    GodColor chooseGodToMove(const GameState& gs, int amount) {
        int bestAction = 0;
        double bestPts = -1e9;
        for (int i = 0; i < 4; i++) {
            auto newGs = gs;
            ownGe_.moveGod(amount, (GodColor) i, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = i;
            }
        }

        return (GodColor) bestAction;
    }

    FlatMap<BookColor, int8_t, 4> chooseBookColorToGet(const GameState& gs, int amount) { 
        FlatMap<BookColor, int8_t, 4> ret = { 0, 0, 0, 0 };
        
        auto tmpGs = gs;
        for (int j = 0; j < amount; j++) {
            int bestAction = 0;
            double bestPts = -1e9;
            for (int i = 0; i < 4; i++) {
                auto newGs = tmpGs;
                ownGe_.awardResources(Resources{ .books = genBook((BookColor) i, 1) }, newGs);
                const auto pts = playOut(newGs, gs.activePlayer);

                if (bestPts < pts) {
                    bestPts = pts;
                    bestAction = i;
                }
            }
            ret[(BookColor) bestAction]++;
            ownGe_.awardResources(Resources{ .books = genBook((BookColor) bestAction, 1) }, tmpGs);
        }

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
        int bestAction = 0;
        double bestPts = -1e9;
        for (const auto [tile, amnt]: gs.fedTilesAvailable) {
            if (amnt == 0) continue;

            auto newGs = gs;
            ownGe_.awardFedTile(tile, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = tile;
            }
        }

        return (FedTileOrigin) bestAction;
    }

    TechTile chooseTechTile(const GameState& gs) {
        const auto& ps = gs.players[gs.activePlayer];

        int bestAction = 0;
        double bestPts = -1e9;
        for (int i = 0; i < 12; i++) {
            TechTile tile = (TechTile) i;
            if (ps.techTiles[tile]) continue;

            auto newGs = gs;
            ownGe_.awardTechTile(tile, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = i;
            }
        }

        return (TechTile) bestAction;
    }

    int8_t choosePlaceToSpade(const GameState& gs, int amount, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        
        return possiblePos[0];
    }

    int8_t choosePlaceForBridge(const GameState& gs, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos[0];
    }

    int8_t choosePlaceToBuildForFree(const GameState& gs,  Building building, bool isNeutral, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;

        int8_t bestAction = 0;
        double bestPts = -1e9;
        for (const auto pos: possiblePos) {
            auto newGs = gs;
            ownGe_.buildForFree(pos, building, isNeutral, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }

        return bestAction;
    }

    int8_t chooseBuildingToConvertForFree(const GameState& gs, Building building, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;

        int8_t bestAction = 0;
        double bestPts = -1e9;
        for (const auto pos: possiblePos) {
            auto newGs = gs;
            ownGe_.upgradeBuilding(pos, building, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }

        return bestAction;
    }

private:
    double evalPs(const GameState& gs, int pIdx) const;

    void genChildren(MctsNode& node) const;
    MctsNode* goBottom(MctsNode& node, int stopRound) const;
    Action buildMcTree(const GameState& gs) const;
    void advanceToNextNode(const Action& action, GameState& gs) const;


    int steps_ = 10000;
    int roundsDepth_ = 1;
    int maxDepth_ = 10;

    AllScoreWeights allScoreWeights_;
    std::unique_ptr<IBot> greedyBot_;

    GameEngine ownGe_;
};
