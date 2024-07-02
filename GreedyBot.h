#pragma once

#include "Bot.h"
#include "GameEngine.h"
#include "Serialize.h"
#include "StaticData.h"
#include "Utils.h"

#include <array>
#include <iostream>
#include <random>

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
        int bestAction = -1;
        double bestPts = std::numeric_limits<double>::lowest();
        for (const auto& [action, _] : enumerate(gs.boosters)) {
            auto newGs = gs;
            ownGe_.awardBooster(action, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = action;
            }
        }
        assert(bestAction >= 0);

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

        assert(!actions.empty());
        auto bestAction = actions.front();
        double bestPts = std::numeric_limits<double>::lowest();
        for (const auto action : actions) {
            // if (action.type == ActionType::Pass) continue;

            double pts = evalAction(gs, action);

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
        int bestAction = -1;
        double bestPts = std::numeric_limits<double>::lowest();
        for (int i = 0; i < 4; i++) {
            auto newGs = gs;
            ownGe_.moveGod(amount, (GodColor) i, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = i;
            }
        }
        // if (bestAction < 0) {
        //     for (int i = 0; i < 4; i++) {
        //         auto newGs = gs;
        //         ownGe_.moveGod(amount, (GodColor) i, newGs);
        //         const auto pts = playOut(newGs, gs.activePlayer);

        //         if (bestPts < pts) {
        //             bestPts = pts;
        //             bestAction = i;
        //         }
        //     }
        // }

        assert(bestAction >= 0);

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
        int bestAction = -1;
        double bestPts = std::numeric_limits<double>::lowest();
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
        assert(bestAction >= 0);
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

        int minSpades = 4;
        std::vector<int8_t> poses;
        const auto myColor = gs.staticGs->playerColors[gs.activePlayer];
        for (const auto& pos : possiblePos) {
            const auto needed = spadesNeeded(myColor, gs.field().type[pos]);
            if (needed < minSpades) {
                minSpades = needed;
                poses.clear();
            }
            if (needed == minSpades) {
                poses.push_back(pos);
            }
        }

        int bestAction = -1;
        double bestPts = std::numeric_limits<double>::lowest();
        for (const auto& pos : poses) {
            auto newGs = gs;
            ownGe_.terraform(pos, amount, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }
        assert(bestAction >= 0);
        return bestAction;
    }

    int choosePlaceForBridge(const GameState& gs, const std::vector<int>& possiblePos) {
        if (possiblePos.empty()) return -1;

        int bestAction = -1;
        double bestPts = std::numeric_limits<double>::lowest();
        for (const auto& pos : possiblePos) {
            auto newGs = gs;
            ownGe_.buildBridge(pos, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }
        assert(bestAction >= 0);
        return bestAction;
    }

    int8_t choosePlaceToBuildForFree(const GameState& gs,  Building building, bool isNeutral, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        int bestAction = -1;
        double bestPts = std::numeric_limits<double>::lowest();
        for (const auto pos: possiblePos) {
            auto newGs = gs;
            ownGe_.buildForFree(pos, building, isNeutral, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }
        assert(bestAction >= 0);

        return bestAction;
    }

    int8_t chooseBuildingToConvertForFree(const GameState& gs, Building building, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        int bestAction = -1;
        double bestPts = std::numeric_limits<double>::lowest();
        for (const auto pos : possiblePos) {
            auto newGs = gs;
            ownGe_.upgradeBuilding(pos, building, newGs);
            const auto pts = playOut(newGs, gs.activePlayer);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = pos;
            }
        }
        assert(bestAction >= 0);

        return bestAction;
    }

private:
    double playOut(const GameState& gs, int pIdx) {
        return evalPs(gs, pIdx);
    }

    double evalAction(const GameState& gs, Action action) const {
        const auto& ps = gs.players[gs.activePlayer];

        const auto& res = ps.resources;
        const auto& curWeights = allScoreWeights_[gs.round];

        const auto evalResources = [&curWeights](const IncomableResources& res) {
            return res.anyBook * curWeights.totalBooks +
                res.anyGod * curWeights.godMove +
                res.cube * curWeights.cube +
                res.gold * curWeights.gold +
                res.humans * curWeights.humans +
                res.manaCharge * curWeights.manaCharge +
                res.spades * curWeights.spades +
                res.winPoints * curWeights.winPoints;
        };
        const auto evalButton = [&curWeights, evalResources, &ps](int buttonOriginIdx) {
            const auto& button = StaticData::buttonOrigins()[buttonOriginIdx];
            double ret = 0.0;
            ret += evalResources(button.resources);

            switch (button.special) {
            case ButtonActionSpecial::BuildBridge: {
                break;
            }
            case ButtonActionSpecial::FiraksButton: {
                ret += curWeights.scorePerBuilding[SC(Building::Guild)] - curWeights.scorePerBuilding[SC(Building::Laboratory)];
                break;
            }
            case ButtonActionSpecial::UpgradeMine: {
                ret += curWeights.scorePerBuilding[SC(Building::Guild)] - curWeights.scorePerBuilding[SC(Building::Mine)];
                break;
            }
            case ButtonActionSpecial::WpForGuilds2: {
                ret += 2 * curWeights.winPoints * ps.countBuildings(Building::Guild);
                break;
            }
            case ButtonActionSpecial::None: {
                break;
            }
            };

            return ret;
        };

        switch (action.type) {
        case ActionType::UpgradeBuilding: {
            const auto pos = action.param1;
            Building newType;
            double additionalScore = 0.0;
            
            if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Guild) {
                if (action.param2 >= 0) {
                    newType = Building::Palace;
                    additionalScore += curWeights.scorePerPalaceIdx[action.param2];
                }
                else {
                    newType = Building::Laboratory;
                    additionalScore = curWeights.scorePerTech[-1 - action.param2];
                }
            }
            else if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Mine) {
                newType = Building::Guild;
            }
            else if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Laboratory) {
                newType = Building::Academy;
                additionalScore = curWeights.scorePerTech[action.param2];
            }

            return curWeights.scorePerBuilding[SC(newType)] - curWeights.scorePerBuilding[SC(gs.field().building[pos].type)] + additionalScore;
        }
        case ActionType::TerraformAndBuild: {
            return curWeights.scorePerBuilding[SC(Building::Mine)];
        }
        case ActionType::UpgradeNav: {
            return curWeights.navLevel[ps.navLevel + 1] - curWeights.navLevel[ps.navLevel];
            break;
        }
        case ActionType::UpgradeTerraform: {
            return curWeights.tfLevel[ps.tfLevel + 1] - curWeights.tfLevel[ps.tfLevel];
            break;
        }
        case ActionType::GetInnovation: {
            return curWeights.scorePerInnovation[action.param1];
        }
        case ActionType::PutManToGod: {
            return curWeights.godMove * action.param2;
        }
        case ActionType::BookMarket: {
            const auto& bookAction = gs.bookActions[action.param1];
            return evalButton(bookAction.buttonOrigin) - curWeights.totalBooks * bookAction.bookPrice;
        }

        case ActionType::ActivateAbility: {
            if (action.param1 < 0) {
                return evalButton(ps.boosterButton.buttonOrigin);
            } else {
                return evalButton(ps.buttons[action.param1].buttonOrigin);
            }
            break;
        }

        case ActionType::Market: {
            auto& marketAction = gs.marketActions[action.param1];
            return evalButton(marketAction.buttonOrigin) - curWeights.manaCharge * 2 * marketAction.manaPrice;
        }

        case ActionType::Annex: {
            return curWeights.totalPower;
        }

        case ActionType::Pass: {
            return 0;
        }
        default:
            assert(false);
        }

        assert(false);
        return -1;
    }

    double evalPs(const GameState& gs, int pIdx) {
        return allScoreWeights_[gs.round].dot(gs.toFeatures(pIdx, ownGe_));
    }

    AllScoreWeights allScoreWeights_;

    GameEngine ownGe_;
};
