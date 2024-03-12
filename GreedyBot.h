#pragma once

#include "Bot.h"
#include "GameEngine.h"
#include "Serialize.h"
#include "StaticData.h"
#include "Utils.h"

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

    double spades = 0;
    double manaCharge = 0;

    double goldIncome = 0;
    double cubeIncome = 0;
    double humansIncome = 0;
    double godsIncome = 0;
    double booksIncome = 0;
    double winPointsIncome = 0;
    double manaIncome = 0;

    double targetGod = 0;
    double godMove = 0;

    double totalPower = 0;
    double scorePerBuilding[7] = {0};
    double scorePerPalaceIdx[17] = { 0 };
    double scorePerTech[12] = { 0 };
    double scorePerInnovation[18] = { 0 };
    double scorePerButtonSpecial[4] = { 0 };

    double navLevel[4] = {0};
    double tfLevel[3] = {0};

    double reachableHexes[4] = {0}; // per terraforms, 0 = native
};

using AllScoreWeights = std::array<ScoreWeights, 7>; // per round, at round's start

class GreedyBot: public IBot {
public:
    GreedyBot(AllScoreWeights allScoreWeights)
        : allScoreWeights_(allScoreWeights)
        , ownGe_({ this, this })
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

        auto bestAction = actions.front();
        double bestPts = -1e9;
        for (const auto& action : actions) {
            auto newGs = gs;
            ownGe_.doAction(action, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

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
        for (int i = 0; i < 12; i++) {
            TechTile tile = (TechTile) i;
            if (!ps.techTiles[tile]) return tile;
        }
        assert(false);
        return (TechTile) -1;
    }

    int8_t choosePlaceToSpade(const GameState& gs, int amount, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;

        int bestAction = 0;
        double bestPts = -1e9;
        for (const auto& pos : possiblePos) {
            auto newGs = gs;
            ownGe_.terraform(pos, amount, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }
        return bestAction;
    }

    int8_t choosePlaceForBridge(const GameState& gs, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;

        int bestAction = 0;
        double bestPts = -1e9;
        for (const auto& pos : possiblePos) {
            auto newGs = gs;
            ownGe_.buildBridge(pos, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }
        return bestAction;
    }

    int8_t choosePlaceToBuildForFree(const GameState& gs,  Building building, bool isNeutral, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        int8_t bestAction = 0;
        double bestPts = -1e9;
        for (const auto pos : possiblePos) {
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
        for (const auto pos : possiblePos) {
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
    double playOut(GameState& gs, int pIdx) {
        return evalPs(gs, pIdx);
    }

    double evalPs(const GameState& gs, int pIdx) {
        const auto ap = gs.activePlayer;
        GameState* hackedGs = const_cast<GameState*>(&gs);
        hackedGs->activePlayer = pIdx;
        const auto& ps = gs.players[pIdx];

        double ret = 0.0;

        const auto& res = ps.resources;
        const auto& curWeights = allScoreWeights_[gs.round];

        ret += res.gold * curWeights.gold;
        ret += res.cube * curWeights.cube;
        ret += res.humans * curWeights.humans;
        ret += sum(res.books.values()) * curWeights.totalBooks;
        ret += sum(res.gods.values()) * curWeights.totalGods;
        ret += res.winPoints * curWeights.winPoints;

        ret += ps.additionalIncome.gold * curWeights.goldIncome;
        ret += ps.additionalIncome.cube * curWeights.cubeIncome;
        ret += ps.additionalIncome.humans * curWeights.humansIncome;
        ret += ps.additionalIncome.anyBook * curWeights.booksIncome;
        ret += ps.additionalIncome.anyGod * curWeights.godsIncome;
        ret += ps.additionalIncome.manaCharge * curWeights.manaIncome;
        
        const auto hexes = ownGe_.someHexes(true, false, gs, 0, 0);
        const auto color = gs.staticGs.playerColors[pIdx];
        std::array<int, 4> tfs = {{0}};
        for (const auto pos: hexes) {
            tfs[spadesNeeded(gs.field().type[pos], color)]++;
        }
        for (int i = 0; i < 4; i++) {
            ret += curWeights.reachableHexes[i] * tfs[i];
        }

        ret += curWeights.navLevel[ps.navLevel];
        ret += curWeights.tfLevel[ps.tfLevel];

        if (gs.round < 5) {
            ret += curWeights.targetGod * res.gods[StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound[gs.round]].god];
        }

        hackedGs->activePlayer = ap;
        return ret;
    }

    AllScoreWeights allScoreWeights_;

    GameEngine ownGe_;
};
