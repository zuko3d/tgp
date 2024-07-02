#pragma once

#include "Bot.h"
#include "GameEngine.h"
#include "Serialize.h"
#include "StaticData.h"
#include "Utils.h"

#include <array>
#include <iostream>
#include <random>

class PassingBot: public IBot {
public:
    PassingBot() { }

    Race chooseRace(const GameState& gs, const std::vector<Race>& races) {
        return races[0];
    }

    TerrainType chooseTerrainType(const GameState& gs, const std::vector<TerrainType>& colors) {
        return colors[0];
    }
    int chooseRoundBooster(const GameState& gs) {
        return 0;
    }

    FullAction chooseAction(const GameState& gs, const std::vector<Action>& actions) {
        return FullAction{
            .preAction = {},
            .action = Action{
                .type = ActionType::Pass
            },
            .postAction = {}
        };
    }

    GodColor chooseGodToMove(const GameState& gs, int amount) {
        return (GodColor) 0;
    }

    FlatMap<BookColor, int8_t, 4> chooseBookColorToGet(const GameState& gs, int amount) { 
        FlatMap<BookColor, int8_t, 4> ret = { (int8_t) amount, 0, 0, 0 };
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
            if (amnt == 0) continue;

            return tile;
        }
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
        return possiblePos.front();
    }

    int choosePlaceForBridge(const GameState& gs, const std::vector<int>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos.front();
    }

    int8_t choosePlaceToBuildForFree(const GameState& gs,  Building building, bool isNeutral, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos.front();
    }

    int8_t chooseBuildingToConvertForFree(const GameState& gs, Building building, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos.front();
    }
};
