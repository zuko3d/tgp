#pragma once

#include "Field.h"
#include "FlatMap.h"
#include "Player.h"
#include "ResizableArray.h"
#include "StaticData.h"
#include "Types.h"

#include <array>
#include <memory>

struct ScoreWeights {
    void initRandomly(std::default_random_engine& rng) {
        double* weightsPtr = (double*) this;
        for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
            weightsPtr[pos] = (rng() % 100) / 1000.0;
        }
    }

    void normalize() {
        double* thisPtr = const_cast<double*>((const double*) this);
        double sum = 0.0;
        for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
            sum += thisPtr[pos];
        }
        sum /= 100.0;
        for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
            thisPtr[pos] /= sum;
        }
    }

    double dot(const ScoreWeights& weights) const {
        double ret = 0.0;
        const double* thisPtr = (const double*) this;
        const double* w2 = (const double*) &weights;
        for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
            ret += thisPtr[pos] * w2[pos];
        }

        return ret;
    }

    ScoreWeights scale(double x) const {
        ScoreWeights ret;
        const double* thisPtr = (const double*) this;
        double* retPtr = (double*) &ret;
        for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
            retPtr[pos] = thisPtr[pos] * x;
        }

        return ret;
    }

    void operator+=(const ScoreWeights& rhs) {
        double* thisPtr = const_cast<double*>((const double*)this);
        const double* w2 = (const double*) &rhs;
        for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
            thisPtr[pos] += w2[pos];
        }
    }

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
    double totalBuildings = 0;

    std::array<double, 7> scorePerBuilding = {0};
    std::array<double, 17> scorePerPalaceIdx = { 0 };
    std::array<double, 12> scorePerTech = { 0 };
    std::array<double, 18> scorePerInnovation = { 0 };

    std::array<double, 4> navLevel = {0, 0, 0, 0};
    std::array<double, 3> tfLevel = {0, 0, 0};

    std::array<double, 4> reachableHexes = {0, 0, 0, 0}; // per terraforms, 0 = native
};

using AllScoreWeights = std::array<ScoreWeights, 7>; // per round, at round's start
using GsFeatures = ScoreWeights;

struct StaticGameState {
    // std::array<int, 5> roundBoosters; // origin idx
    int lastRoundBonus;
    std::array<int, 6> bonusByRound; // origin idx
    std::array<Race, 2> playerRaces = { Race::None, Race::None };
    std::array<TerrainType, 2> playerColors = { TerrainType::None, TerrainType::None };
    std::array<std::array<TechTile, 3>, 4> techTiles;
    FlatMap<TechTile, Resources, 12> bookAndGodPerTech;
    std::array<Palace, 5> palaces;
    FlatMap<GodColor, int8_t, 4> neutralGods;
};

struct GameEngine;

struct GameState
{
    GameState clone() {
        GameState ret = *this;
        ret.cache = std::shared_ptr<PrecalcCache>(new PrecalcCache(*cache));
        return ret;
    }

    uint8_t activePlayer;
    uint8_t round = 0;

    ResizableArray<RoundBoosterOnBoard, 5> boosters;
    
    ResizableArray<uint8_t, 2> playersOrder;

    std::array<PlayerState, 2> players;

    FlatMap<FedTileOrigin, uint8_t, 7> fedTilesAvailable;
    std::array<Innovation, 6> innovations;
    ResizableArray<uint8_t, 4> palacesAvailable;

    int fieldStateIdx;
    const Field& field() const {
        return cache->fieldByState_[fieldStateIdx];
    }
    Field& field() {
        return cache->fieldByState_[fieldStateIdx];
    }

    GsFeatures toFeatures(int playerIdx, const GameEngine& ge) const;

    FlatMap<GodColor, ResizableArray<uint8_t, 3>, 4> humansOnGods;

    std::array<BookButton, 3> bookActions;
    std::array<MarketButton, 6> marketActions;

    GamePhase phase = GamePhase::Upkeep;

    std::shared_ptr<StaticGameState> staticGs = std::shared_ptr<StaticGameState>(new StaticGameState());
    std::shared_ptr<PrecalcCache> cache;
};
