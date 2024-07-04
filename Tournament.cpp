#include "Tournament.h"

#include "GameEngine.h"
#include "Timer.h"
#include "Utils.h"

#include <omp.h>
#include <fstream>
#include <iostream>

std::vector<double> GameResult::trueSkill(const std::vector<GameResult> &gr) {
    int nBots = 0;
    for (const auto& r: gr) {
        for (const auto& idx: r.botIndices) {
            nBots = std::max(idx, nBots);
        }
    }
    nBots++;

    std::vector<std::vector<double>> games(nBots);
    for (const auto& res: gr) {
        for (const auto& [idx, p]: enumerate(res.botIndices)) {
            games.at(p).push_back(res.winPoints.at(idx));
        }
    }

    std::vector<double> ret(nBots);
    for (const auto& [idx, g]: enumerate(games)) {
        ret.at(idx) = mean(g) - 3 * variance(g) / sqrt(g.size());
    }

    return ret;
}

std::vector<double> GameResult::avgWinPoints(const std::vector<GameResult> &gr) {
    std::vector<double> ret;
    std::vector<double> games;
    int nBots = 0;
    for (const auto& r: gr) {
        for (const auto& idx: r.botIndices) {
            nBots = std::max(idx, nBots);
        }
    }
    nBots++;

    ret.resize(nBots, 0.0);
    games.resize(nBots, 0.0);
    for (const auto& res: gr) {
        for (const auto& [idx, p]: enumerate(res.botIndices)) {
            games.at(p)++;
            ret.at(p) += res.winPoints.at(idx);
        }
    }

    for (const auto& [idx, g]: enumerate(games)) {
        ret.at(idx) /= g;
    }

    return ret;
}

GameResult Tournament::playSingleGame(const std::vector<IBot*>& bots, uint32_t seed, bool withLogs) {
    Timer timer;

    GameEngine ge(bots, false, false, statsParams);
    GameState gs;
    std::default_random_engine g{seed};
    ge.initializeRandomly(gs, g, {Race::Inventors, Race::Monks}, {TerrainType::Wasteland, TerrainType::Desert});
    ge.playGame(gs);
    const std::vector<int> winPoints = {
        gs.players[0].resources.winPoints,
        gs.players[1].resources.winPoints,
    };

    const auto curTrainStats = ge.moveTrainStats();

#pragma omp critical
    std::copy(curTrainStats.begin(), curTrainStats.end(), std::back_inserter(trainStats));

    // std::cout << "game time: " << timer.elapsedMilliSeconds() << std::endl;
    return GameResult{
		.winner = (int) std::distance(winPoints.begin(), std::max_element(winPoints.begin(), winPoints.end())),
		.winPoints = winPoints,
		.botIndices = {0, 1}
	};
}

std::vector<GameResult> Tournament::playAllInAll(const std::vector<IBot*>& bots, int repeat) {
	std::vector<GameResult> results;

// #ifndef DEBUG
// #pragma omp parallel for schedule(dynamic)
// #endif

    omp_set_num_threads(8);
    Timer timer;
	for (int p0 = 0; p0 < bots.size(); p0++) {
#pragma omp parallel for schedule(dynamic)
		for (int seed = 0; seed < repeat; seed++) {
			// std::cout << "seed: " << seed << std::endl;
			// std::cout << ".";
			// std::cout.flush();
            const auto seed_ = seed;
			for (int p1 = p0 + 1; p1 < bots.size(); p1++) {
				{
                    Timer timerGame;
                    auto result = playSingleGame({bots.at(p0), bots.at(p1)}, seed_);
                    result.botIndices = {p0, p1};

                    // std::cout << "01, seed " << seed_ << ", time: " << timerGame.elapsedMilliSeconds() << ", wp: " << result.winPoints.at(result.winner) << std::endl;
                    // std::cout << result.winPoints[0] << "\t" << result.winPoints[1] << std::endl;

#pragma omp critical
					results.push_back(result);

				}
				{
                    Timer timerGame;
					auto result = playSingleGame({bots.at(p1), bots.at(p0)}, seed_);
                    result.botIndices = {p1, p0};

                    // std::cout << "10, seed " << seed_ << ", time: " << timerGame.elapsedMilliSeconds() << ", wp: " << result.winPoints.at(result.winner) << std::endl;
                    // std::cout << result.winPoints[0] << "\t" << result.winPoints[1] << std::endl;

#pragma omp critical
					results.push_back(result);
				}
			}
			// if (timer.elapsedSeconds() > 20) {
			// 	std::cout << "!";
			// 	std::cout.flush();
			// 	break;
			// }
		}
	}

    std::cout << "Elapsed: " << timer.elapsedMilliSeconds() << std::endl;

	// std::cout << std::endl;
	return results;
}
