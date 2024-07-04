#pragma once

#include "Bot.h"
#include "GameEngine.h"

#include <vector>

struct GameResult
{
    static std::vector<double> avgWinPoints(const std::vector<GameResult> &gr);
    // static std::vector<double> winrates(const std::vector<GameResult> &gr);
    // static std::vector<double> percentileWinPoints(const std::vector<GameResult> &gr, double pct);
    static std::vector<double> trueSkill(const std::vector<GameResult> &gr);

    // static std::vector<PlayerIngameStats> transposeAndFilter(const std::vector<GameResult>& results, int wpThreshold = 0);

	int winner;
	std::vector<int> winPoints;
    std::vector<int> botIndices;
};

class Tournament {
public:
    GameResult playSingleGame(const std::vector<IBot*>& bots, uint32_t seed, bool withLogs = false);
    std::vector<GameResult> playAllInAll(const std::vector<IBot*>& bots, int repeat);

    std::vector<TrainStats> trainStats;
    std::optional<StatsParams> statsParams;

    std::default_random_engine rng{1};
};