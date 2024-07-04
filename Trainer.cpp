#include "Trainer.h"

#include "GreedyBot.h"
#include "MctsBot.h"
#include "PassingBot.h"
#include "serialize.h"
#include "Tournament.h"
#include "Timer.h"

void Trainer::train(AllScoreWeights& weights1, int epochs) {
    for (int iteration = 0; iteration < 1000; iteration++) {
        std::cout << "train iteration " << iteration << std::endl;
        for (int round = 5; round >= 0; round--) {
            Tournament t;
            std::cout << "round " << round << std::endl;

            t.statsParams = StatsParams { .initialRound = round, .targetRound = round + 1, .allScoreWeights = weights1 };
            MctsBot bot1(new GreedyBot(weights1), weights1, 50, 3, 15);
            PassingBot bot2;

            // GreedyBot bot1(weights1);
            // GreedyBot bot2(weights2);

            const auto results = t.playAllInAll({&bot1, &bot2}, epochs);
            
            const auto avgWinPoints = GameResult::avgWinPoints(results);
            std::cout << "avgWinPoints: " << avgWinPoints.front() << std::endl;

            const auto trueSkill = GameResult::trueSkill(results);
            std::cout << "trueSkill: " << trueSkill.front() << std::endl;

            const auto best = std::max_element(results.begin(), results.end(), [] (const GameResult& op1, const GameResult& op2) {
                return op1.winPoints[op1.winner] < op2.winPoints[op2.winner];
            });
            std::cout << "highscore:" << best->winPoints.at(best->winner) << std::endl;

            constexpr double lr = 1e-3;

            std::cout << "sgd, t.trainStats size: " << t.trainStats.size() << std::endl;

            // for (auto& stat: t.trainStats) {
            //     stat.gsFeatures.normalize();
            // }

            // double th = 0.0;
            // for (const auto& stat: t.trainStats) {
            //     th += pow(stat.finalScore, 1.0 / 3.0);
            // }
            // th /= t.trainStats.size();
            // th = pow(th, 3.0);
            // t.trainStats.resize(std::distance(
            //     t.trainStats.begin(),
            //     std::remove_if(t.trainStats.begin(), t.trainStats.end(), [th] (const TrainStats& op) { return op.finalScore < th; })
            // ));

            const auto updateWeights = [round, &t] (AllScoreWeights& weights) {
                double mse = 0;
                Timer timer;
                double* wPtr = (double*) &(weights[round]);
                for (int sgdEpoch = 0; sgdEpoch < 100000; sgdEpoch++) {
                    mse = 0;
                    weights[round] = weights[round].scale(0.9999);
                    for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
                        if (wPtr[pos] > 0) wPtr[pos] -= 1e-5;
                            else wPtr[pos] += 1e-5;
                    }

                    for (const auto& stat: t.trainStats) {
                        double diff = stat.finalScore - stat.gsFeatures.dot(weights[round]);
                        mse += diff * diff;
                        if (diff != 0) diff /= std::fabs(diff);
                        const auto delta = stat.gsFeatures.scale(diff * lr);

                        weights[round] += delta;

                        // weights[stat.round].winPoints = 1.0;
                        // weights[stat.round].winPointsIncome = 5.0 - stat.round;
                    }

                    mse = std::sqrt(mse) / t.trainStats.size();
                    // std::cout << "sgdEpoch: " << sgdEpoch <<  "\tmse: " << mse << std::endl;
                    // weights[round].normalize();
                }

                double norm = 0.0;
                for (int pos = 0; pos < sizeof(ScoreWeights) / 8; pos++) {
                    wPtr[pos] = std::round(wPtr[pos] * 1000.0) / 1000.0;
                    norm += std::fabs(wPtr[pos]);
                }
                std::cout << "mse: " << mse << ", norm: " << norm << ", sgd elapsed msec: " << timer.elapsedMilliSeconds() << std::endl;

                weights[round].manaCharge = (7.0 * weights[round].gold + 2.0 * weights[round].cube) / 16.0;
                weights[round].spades = weights[round].cube;
            };

            updateWeights(weights1);

            std::cout << toJson(weights1) << std::endl;
        }
    }
}

void Trainer::train2(AllScoreWeights& weights, int epochs) {
    for (int iteration = 0; iteration < 100; iteration++) {
        Tournament t;
        std::cout << "train iteration " << iteration << std::endl;

        t.statsParams = StatsParams { .initialRound = 4, .targetRound = -1, .allScoreWeights = weights };
        MctsBot bot1(new GreedyBot(weights), weights, 50, 2, 15);
        PassingBot bot2;

        const auto results = t.playAllInAll({&bot1, &bot2}, epochs);
        
        const auto avgWinPoints = GameResult::avgWinPoints(results);
        std::cout << "avgWinPoints: " << avgWinPoints.front() << std::endl;

        const auto trueSkill = GameResult::trueSkill(results);
        std::cout << "trueSkill: " << trueSkill.front() << std::endl;

        const auto best = std::max_element(results.begin(), results.end(), [] (const GameResult& op1, const GameResult& op2) {
            return op1.winPoints[op1.winner] < op2.winPoints[op2.winner];
        });
        std::cout << "highscore:" << best->winPoints.at(best->winner) << std::endl;

        std::cout << "sgd, t.trainStats size: " << t.trainStats.size() << std::endl;

        // for (auto& stat: t.trainStats) {
        //     stat.gsFeatures.normalize();
        // }

        double lr = 1e-2 / t.trainStats.size();

        double mse = 0;
        Timer timer;
        double* wPtr = (double*) &(weights);
        for (int sgdEpoch = 0; sgdEpoch < 100000; sgdEpoch++) {
            if (sgdEpoch % 1000 == 0) {
                std::default_random_engine rng(sgdEpoch);
                rshuffle(t.trainStats, rng);
            }
            mse = 0;
            for (auto& w: weights) {
                w.scale(0.999);
            }
            for (int pos = 0; pos < sizeof(AllScoreWeights) / 8; pos++) {
                if (wPtr[pos] > 0) wPtr[pos] -= 1e-4;
                    else wPtr[pos] += 1e-4;
            }

            for (const auto& stat: t.trainStats) {
                double diff = stat.finalScore - stat.gsFeatures.dot(weights[stat.round]);
                mse += diff * diff;
                if (diff != 0) diff /= std::fabs(diff);
                const auto delta = stat.gsFeatures.scale(diff * lr);

                weights[stat.round] += delta;

                weights[stat.round].winPoints = 1.0;
                weights[stat.round].winPointsIncome = 5.0 - stat.round;
            }
            
            for (auto& w: weights) {
                w.manaCharge = (7.0 * w.gold + 2.0 * w.cube) / 16.0;
                w.spades = w.cube;
                w.godMove = w.totalGods;
            }
            mse = std::sqrt(mse) / t.trainStats.size();
            // std::cout << "sgdEpoch: " << sgdEpoch <<  "\tmse: " << mse << std::endl;
            // weights[round].normalize();
        }

        weights.back().winPoints = 1.0;
        double norm = 0.0;
        for (int pos = 0; pos < sizeof(AllScoreWeights) / 8; pos++) {
            wPtr[pos] = std::round(wPtr[pos] * 1000.0) / 1000.0;
            norm += std::fabs(wPtr[pos]);
        }
        std::cout << "mse: " << mse << ", norm: " << norm << ", sgd elapsed msec: " << timer.elapsedMilliSeconds() << std::endl;

        std::cout << toJson(weights) << std::endl;
    }
}
