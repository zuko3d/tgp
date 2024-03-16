#include "GameEngine.h"
#include "RandomBot.h"
#include "WebUiBot.h"

#include "GreedyBot.h"
#include "ScoringBot.h"
#include "MctsBot.h"
#include "Tournament.h"
#include "serialize.h"

#include <memory>
#include <random>
#include <iostream>

int main() {
    AllScoreWeights allScoreWeights {
        ScoreWeights{ // round 0
            .gold = 1,
            .cube = 2.8,
            .humans = 3.3,
            .totalBooks = 3.8,
            .totalGods = 1.5,
            .winPoints = 0.5,

            .manaCharge = 0.2,

            .goldIncome = 5.2,
            .cubeIncome = 15,
            .humansIncome = 18,
            .godsIncome = 7.5,
            .booksIncome = 20,
            .winPointsIncome = 5,
            .manaIncome = 5,

            .targetGod = 1.5,

            .totalPower = 1,
            .scorePerBuilding = { 1, 1, 1, 1, 1, 1, 1 },

            .navLevel = { 0, 2, 4, 10 },
            .tfLevel = { 0, 2, 4 },

            .reachableHexes = { 10, 5, 2, 1 }
        },

        ScoreWeights{ // round 1
            .gold = 1,
            .cube = 3,
            .humans = 3.5,
            .totalBooks = 4,
            .totalGods = 1.5,
            .winPoints = 0.6,

            .goldIncome = 4,
            .cubeIncome = 12,
            .humansIncome = 16,
            .godsIncome = 6,
            .booksIncome = 16,
            .winPointsIncome = 4,
            .manaIncome = 4,

            .targetGod = 1.5,

            .totalPower = 1,
            .scorePerBuilding = { 1, 1, 1, 1, 1, 1, 1 },

            .navLevel = { 0, 2, 4, 10 },
            .tfLevel = { 0, 2, 4 },

            .reachableHexes = { 10, 5, 2, 1 }
        },

        ScoreWeights{ // round 2
            .gold = 1,
            .cube = 3,
            .humans = 3.5,
            .totalBooks = 4,
            .totalGods = 1.5,
            .winPoints = 0.7,

            .goldIncome = 3,
            .cubeIncome = 9,
            .humansIncome = 12,
            .godsIncome = 4.5,
            .booksIncome = 12,
            .winPointsIncome = 3,
            .manaIncome = 3,

            .targetGod = 1.5,

            .totalPower = 1,
            .scorePerBuilding = { 1, 1, 1, 1, 1, 1, 1 },

            .navLevel = { 0, 2, 4, 10 },
            .tfLevel = { 0, 2, 4 },

            .reachableHexes = { 10, 5, 2, 1 }
        },
        ScoreWeights{ // round 3
            .gold = 1,
            .cube = 3,
            .humans = 3.5,
            .totalBooks = 4,
            .totalGods = 1.5,
            .winPoints = 0.8,

            .goldIncome = 2,
            .cubeIncome = 6,
            .humansIncome = 9,
            .godsIncome = 3,
            .booksIncome = 10,
            .winPointsIncome = 2,
            .manaIncome = 2,

            .targetGod = 1.5,

            .totalPower = 1,
            .scorePerBuilding = { 1, 1, 1, 1, 1, 1, 1 },

            .navLevel = { 0, 2, 4, 10 },
            .tfLevel = { 0, 2, 4 },

            .reachableHexes = { 10, 5, 2, 1 }
        },

        ScoreWeights{ // round 4
            .gold = 1,
            .cube = 3,
            .humans = 3.5,
            .totalBooks = 4,
            .totalGods = 1.5,
            .winPoints = 0.8,

            .goldIncome = 1,
            .cubeIncome = 3,
            .humansIncome = 3,
            .godsIncome = 1.2,
            .booksIncome = 3,
            .winPointsIncome = 1,
            .manaIncome = 1,

            .targetGod = 1.5,

            .totalPower = 1,
            .scorePerBuilding = { 1, 1, 1, 1, 1, 1, 1 },

            .navLevel = { 0, 2, 4, 10 },
            .tfLevel = { 0, 2, 4 },

            .reachableHexes = { 10, 5, 2, 1 }
        },

        ScoreWeights{ // round 5
            .gold = 1,
            .cube = 3,
            .humans = 3.5,
            .totalBooks = 4,
            .totalGods = 1.5,
            .winPoints = 0.5,

            .goldIncome = 0,
            .cubeIncome = 0,
            .humansIncome = 0,
            .godsIncome = 0,
            .booksIncome = 0,
            .winPointsIncome = 0,
            .manaIncome = 0,

            .targetGod = 0,

            .totalPower = 1,
            .scorePerBuilding = { 1, 1, 1, 1, 1, 1, 1 },

            .navLevel = { 0, 2, 4, 10 },
            .tfLevel = { 0, 2, 4 },

            .reachableHexes = { 10, 5, 2, 1 }
        },
        
        ScoreWeights{ // round 6 (End of the game)
            .gold = 0,
            .cube = 0,
            .humans = 0,
            .totalBooks = 0,
            .totalGods = 0,
            .winPoints = 1,

            .goldIncome = 0,
            .cubeIncome = 0,
            .humansIncome = 0,
            .godsIncome = 0,
            .booksIncome = 0,
            .winPointsIncome = 0,
            .manaIncome = 0,

            .targetGod = 0,

            .totalPower = 0,
            .scorePerBuilding = { 0, 0, 0, 0, 0, 0, 0 },

            .navLevel = { 0, 0, 0, 0 },
            .tfLevel = { 0, 0, 0 },

            .reachableHexes = { 0, 0, 0, 0 }
        },
    };

    // AllScoreWeights curBestWeights = allScoreWeights;
    // double lr = 0.2;

    // for (int iter = 0; iter < 10; iter++) {
    //     std::cout << "============================" << std::endl;
    //     std::cout << "iter: " << iter << std::endl;
    //     for (int i = 0; i < 6; i++) {
    //         std::cout << "round " << i << ": " << toJson(curBestWeights.at(i)).dump() << std::endl;
    //     }

    //     for (int pos = 0; pos < sizeof(AllScoreWeights) / 8; pos++) {
    //         std::cout << "pos: " << pos << std::endl;
    //         auto pretender = curBestWeights;
    //         double* pretenderPtr = (double*) &pretender;
    //         bool becameBetter = true;
    //         while (becameBetter) {
    //             becameBetter = false;
    //             if (pretenderPtr[pos] >= lr) {
    //                 pretenderPtr[pos] -= lr;
    //                 auto bot2 = MctsBot(new GreedyBot(allScoreWeights), pretender, 200, 6, 3);
    //                 auto bot3 = MctsBot(new GreedyBot(allScoreWeights), curBestWeights, 200, 6, 3);
    //                 const auto result = Tournament::playAllInAll({ &bot2, &bot3}, 60);
    //                 const auto skills = GameResult::trueSkill(result);
    //                 if (skills[0] > skills[1]) {
    //                     std::cout << "down to new value: " << pretenderPtr[pos] << std::endl;
    //                     for (const auto [idx, score]: enumerate(skills)) {
    //                         std::cout << "player " << idx << " trueskill:\t" << score << std::endl;
    //                     }
    //                     becameBetter = true;
    //                     curBestWeights = pretender;
    //                 }                
    //             }
    //         }
    //         becameBetter = true;
    //         while (becameBetter) {
    //             becameBetter = false;
    //             pretender = curBestWeights;
    //             pretenderPtr[pos] += lr;
    //             auto bot2 = MctsBot(new GreedyBot(allScoreWeights), pretender, 200, 6, 3);
    //             auto bot3 = MctsBot(new GreedyBot(allScoreWeights), curBestWeights, 200, 6, 3);
    //             const auto result = Tournament::playAllInAll({ &bot2, &bot3}, 60);
    //             const auto skills = GameResult::trueSkill(result);
    //             if (skills[0] > skills[1]) {
    //                 std::cout << "up to new value: " << pretenderPtr[pos] << std::endl;
    //                 for (const auto [idx, score]: enumerate(skills)) {
    //                     std::cout << "player " << idx << " trueskill:\t" << score << std::endl;
    //                 }
    //                 becameBetter = true;
    //                 curBestWeights = pretender;
    //             }
    //         }
    //     }

    //     std::cout << "Decay..." << std::endl;
    //     {
    //         double* weightsPtr = (double*) &curBestWeights;
    //         for (int pos = 0; pos < sizeof(AllScoreWeights) / 8; pos++) {
    //             weightsPtr[pos] *= 0.5;
    //         }
    //     }
    // }
    
    // return 0;

    auto webBot = WebUiBot(std::default_random_engine{1});
    auto cBot = MctsBot(new GreedyBot(allScoreWeights), allScoreWeights, 10000, 6, 6);
    Tournament::playSingleGame({ &webBot, &cBot }, 42);
    return 0;
}