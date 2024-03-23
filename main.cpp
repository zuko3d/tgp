#include "GameEngine.h"
#include "RandomBot.h"
#include "WebUiBot.h"

#include "GreedyBot.h"
#include "ScoringBot.h"
#include "MctsBot.h"
#include "Tournament.h"
#include "Serialize.h"

#include <memory>
#include <random>
#include <iostream>

int main() {
    AllScoreWeights allScoreWeights {
        fromJsonStr("{\"booksIncome\":34.379969673434694,\"cube\":8.80117607899706,\"cubeIncome\":68.75993934686939,\"godMove\":90.24742039276607,\"godsIncome\":105.28865712489375,\"gold\":5.7187026147417,\"goldIncome\":126.77613817079042,\"humans\":18.49484078553215,\"humansIncome\":103.13990902030407,\"manaCharge\":88.09867228817639,\"manaIncome\":27.933725359665686,\"navLevel\":[42.974962091793365,90.24742039276606,154.70986353045612,103.13990902030407],\"reachableHexes\":[150.41236732127678,66.61119124227972,68.75993934686939,25.78497725507602],\"scorePerBuilding\":[176.1973445763528,34.379969673434694,307.4374052294834,199.83357372683915,98.84241281112475,8.594992418358673,49.42120640556237],\"scorePerInnovation\":[45.12371019638303,49.42120640556237,23.63622915048635,201.9823218314288,79.50367986981772,17.189984836717347,98.84241281112475,212.72606235437715,126.77613817079042,124.62739006620075,27.933725359665686,156.85861163504578,174.04859647176315,58.016198823921044,154.70986353045612,103.13990902030407,62.31369503310037,49.42120640556237],\"scorePerPalaceIdx\":[163.30485594881478,159.00735973963543,111.73490143866275,62.31369503310037,116.03239764784209,38.67746588261403,206.27981804060815,186.94108509930112,6.446244313769005,79.50367986981772,156.85861163504578,6.446244313769005,141.81737490291812,197.6848256222495,21.487481045896683,53.7187026147417,62.31369503310037],\"scorePerTech\":[182.6435888901218,49.42120640556237,49.42120640556237,139.66862679832843,210.5773142497875,105.28865712489375,45.12371019638303,180.49484078553215,137.51987869373878,139.66862679832843,189.0898332038908,85.94992418358673],\"spades\":195.5360775176598,\"targetGod\":85.94992418358673,\"tfLevel\":[139.66862679832843,126.77613817079042,191.23858130848046],\"totalBooks\":146.11487111209746,\"totalGods\":135.3711305891491,\"totalPower\":45.12371019638303,\"winPoints\":27.933725359665686,\"winPointsIncome\":92.39616849735573}"),
        fromJsonStr("{\"booksIncome\":90.24742039276606,\"cube\":18.18114575243176,\"cubeIncome\":165.45360405340446,\"godMove\":186.94108509930112,\"godsIncome\":92.39616849735573,\"gold\":9.69366470653507,\"goldIncome\":210.5773142497875,\"humans\":15.41236732127678,\"humansIncome\":58.016198823921044,\"manaCharge\":42.974962091793365,\"manaIncome\":165.45360405340446,\"navLevel\":[0.0,98.84241281112475,34.379969673434694,27.933725359665686],\"reachableHexes\":[105.28865712489375,152.56111542586643,62.31369503310039,94.5449166019454],\"scorePerBuilding\":[156.85861163504578,135.37113058914912,398.84241281112475,34.379969673434694,77.35493176522806,21.487481045896683,107.4374052294834],\"scorePerInnovation\":[143.96612300750778,19.338732941307015,40.826213987203694,195.53607751765978,208.4285661451978,111.73490143866275,75.20618366063839,85.94992418358673,169.7511002625838,189.0898332038908,30.082473464255358,206.27981804060815,118.18114575243176,118.18114575243176,105.28865712489375,25.78497725507602,174.04859647176315,197.6848256222495],\"scorePerPalaceIdx\":[100.9911609157144,161.15610784422512,178.34609268094246,118.18114575243176,96.69366470653507,212.72606235437715,40.826213987203694,154.70986353045612,21.487481045896683,193.38732941307015,167.60235215799412,171.89984836717346,68.75993934686939,66.61119124227972,40.826213987203694,36.528717778024365,191.23858130848046],\"scorePerTech\":[62.31369503310037,6.446244313769005,8.594992418358673,154.70986353045612,2.1487481045896684,105.28865712489375,12.89248862753801,17.189984836717347,19.338732941307015,167.60235215799412,124.62739006620075,292.22974222419487],\"spades\":62.31369503310037,\"targetGod\":111.73490143866275,\"tfLevel\":[176.19734457635278,161.15610784422512,85.94992418358673],\"totalBooks\":40.826213987203694,\"totalGods\":152.56111542586643,\"totalPower\":113.88364954325242,\"winPoints\":55.86745071933137,\"winPointsIncome\":186.94108509930112}"),
        fromJsonStr("{\"booksIncome\":225.61855098191518,\"cube\":2.974962091793365,\"cubeIncome\":17.189984836717347,\"godMove\":40.826213987203694,\"godsIncome\":221.32105477273583,\"gold\":16.03239764784209,\"goldIncome\":79.50367986981772,\"humans\":24.2547801324015,\"humansIncome\":210.5773142497875,\"manaCharge\":111.73490143866275,\"manaIncome\":189.0898332038908,\"navLevel\":[40.826213987203694,62.31369503310039,36.528717778024365,25.78497725507602],\"reachableHexes\":[154.70986353045612,156.85861163504578,178.34609268094246,45.12371019638303],\"scorePerBuilding\":[210.5773142497875,150.41236732127678,303.13990902030407,191.23858130848046,55.86745071933137,21.487481045896683,21.487481045896683],\"scorePerInnovation\":[165.45360405340446,109.58615333407307,131.07363437996975,88.09867228817639,176.1973445763528,148.2636192166871,42.974962091793365,85.94992418358673,186.94108509930112,21.487481045896683,2.1487481045896684,111.73490143866275,204.13106993601846,135.3711305891491,77.35493176522806,49.42120640556237,131.07363437996975,184.79233699471146],\"scorePerPalaceIdx\":[81.65242797440739,169.7511002625838,152.56111542586643,85.94992418358673,244.95728392322215,109.58615333407307,148.2636192166871,70.90868745145906,184.79233699471146,154.70986353045612,30.082473464255358,60.164946928510716,313.71722327009155,96.69366470653507,96.69366470653507,79.50367986981772,141.81737490291812],\"scorePerTech\":[79.50367986981772,182.6435888901218,206.27981804060815,253.55227634158084,68.75993934686939,109.58615333407307,53.7187026147417,326.6097118976295,156.85861163504578,217.0235585635565,66.61119124227972,103.13990902030409],\"spades\":47.272458300972694,\"targetGod\":94.5449166019454,\"tfLevel\":[113.88364954325242,189.0898332038908,96.69366470653507],\"totalBooks\":51.56995451015204,\"totalGods\":206.27981804060815,\"totalPower\":159.00735973963543,\"winPoints\":139.66862679832843,\"winPointsIncome\":159.00735973963543}"),
        fromJsonStr("{\"booksIncome\":154.70986353045612,\"cube\":5.041236732127679,\"cubeIncome\":167.60235215799412,\"godMove\":232.06479529568418,\"godsIncome\":169.7511002625838,\"gold\":6.31369503310037,\"goldIncome\":34.379969673434694,\"humans\":4.12371019638303,\"humansIncome\":36.528717778024365,\"manaCharge\":77.35493176522806,\"manaIncome\":73.05743555604873,\"navLevel\":[161.15610784422512,150.41236732127678,128.9248862753801,17.189984836717347],\"reachableHexes\":[25.78497725507602,126.77613817079042,139.66862679832843,73.05743555604873],\"scorePerBuilding\":[118.18114575243176,100.9911609157144,309.58615333407307,182.6435888901218,148.2636192166871,2.1487481045896684,4.297496209179337],\"scorePerInnovation\":[154.70986353045612,17.189984836717347,109.58615333407307,47.2724583009727,58.016198823921044,64.46244313769004,32.23122156884502,66.61119124227972,212.72606235437715,55.86745071933137,92.39616849735573,193.38732941307015,92.39616849735573,66.61119124227972,79.50367986981772,212.72606235437715,184.79233699471146,0.0],\"scorePerPalaceIdx\":[189.0898332038908,66.61119124227972,34.379969673434694,30.082473464255358,17.189984836717347,165.45360405340446,60.164946928510716,83.80117607899706,81.65242797440739,161.15610784422512,85.94992418358673,159.00735973963543,116.03239764784209,0.0,96.69366470653507,75.20618366063839,94.5449166019454],\"scorePerTech\":[51.56995451015204,103.13990902030409,199.83357372683912,150.41236732127678,255.7010244461705,45.12371019638303,0.0,68.75993934686939,92.39616849735573,197.6848256222495,10.743740522948341,4.297496209179337],\"spades\":143.96612300750778,\"targetGod\":98.84241281112475,\"tfLevel\":[85.94992418358673,92.39616849735573,212.72606235437715],\"totalBooks\":249.2547801324015,\"totalGods\":244.95728392322215,\"totalPower\":2.1487481045896684,\"winPoints\":105.28865712489375,\"winPointsIncome\":38.67746588261403}"),
        fromJsonStr("{\"booksIncome\":163.30485594881478,\"cube\":35.3711305891491,\"cubeIncome\":201.9823218314288,\"godMove\":214.8748104589668,\"godsIncome\":38.67746588261403,\"gold\":4.297496209179334,\"goldIncome\":68.75993934686939,\"humans\":22.91604719109452,\"humansIncome\":167.60235215799412,\"manaCharge\":40.826213987203694,\"manaIncome\":38.67746588261403,\"navLevel\":[113.88364954325242,174.04859647176315,206.27981804060815,19.338732941307015],\"reachableHexes\":[64.46244313769004,234.21354340027386,197.6848256222495,88.09867228817639],\"scorePerBuilding\":[8.594992418358673,68.75993934686939,393.38732941307015,79.50367986981772,126.77613817079045,83.80117607899706,79.50367986981772],\"scorePerInnovation\":[178.34609268094246,73.05743555604873,135.37113058914912,96.69366470653507,107.4374052294834,51.56995451015204,83.80117607899706,34.379969673434694,206.27981804060815,197.6848256222495,133.22238248455943,141.81737490291812,135.3711305891491,66.61119124227972,120.32989385702143,12.89248862753801,182.6435888901218,126.77613817079042],\"scorePerPalaceIdx\":[68.75993934686939,90.24742039276606,133.22238248455943,10.743740522948341,98.84241281112475,19.338732941307015,73.05743555604873,10.743740522948341,141.81737490291812,34.379969673434694,120.32989385702143,107.4374052294834,73.05743555604873,85.94992418358673,62.31369503310037,21.487481045896683,171.89984836717346],\"scorePerTech\":[159.00735973963543,169.7511002625838,60.164946928510716,49.42120640556237,165.45360405340446,103.13990902030407,120.32989385702143,109.58615333407307,40.826213987203694,184.79233699471146,178.34609268094246,17.189984836717347],\"spades\":45.12371019638303,\"targetGod\":199.83357372683915,\"tfLevel\":[201.9823218314288,131.07363437996975,73.05743555604873],\"totalBooks\":122.47864196161107,\"totalGods\":212.72606235437715,\"totalPower\":171.89984836717346,\"winPoints\":358.84093346647455,\"winPointsIncome\":124.62739006620075}"),
        fromJsonStr("{\"booksIncome\":2.1487481045896684,\"cube\":8.80117607899706,\"cubeIncome\":204.13106993601846,\"godMove\":55.86745071933135,\"godsIncome\":255.7010244461705,\"gold\":8.65242797440739,\"goldIncome\":120.32989385702143,\"humans\":15.41236732127678,\"humansIncome\":92.39616849735575,\"manaCharge\":176.19734457635278,\"manaIncome\":120.32989385702143,\"navLevel\":[268.59351307370855,197.6848256222495,232.06479529568418,12.89248862753801],\"reachableHexes\":[77.35493176522806,180.49484078553212,103.13990902030407,30.082473464255358],\"scorePerBuilding\":[111.73490143866275,199.83357372683912,374.04859647176315,193.38732941307015,242.80853581863252,58.016198823921044,201.9823218314288],\"scorePerInnovation\":[159.00735973963543,174.04859647176315,128.9248862753801,6.446244313769005,38.67746588261403,159.00735973963543,47.2724583009727,120.32989385702143,53.7187026147417,107.4374052294834,169.7511002625838,70.90868745145906,148.2636192166871,182.6435888901218,191.23858130848046,103.13990902030407,96.69366470653507,30.082473464255358],\"scorePerPalaceIdx\":[163.30485594881478,156.85861163504578,182.6435888901218,171.89984836717346,131.07363437996975,186.94108509930112,92.39616849735573,182.6435888901218,124.62739006620075,6.446244313769005,21.487481045896683,68.75993934686939,141.81737490291812,120.32989385702143,116.03239764784209,92.39616849735573,34.379969673434694],\"scorePerTech\":[253.55227634158084,193.38732941307012,94.5449166019454,195.5360775176598,116.03239764784209,17.189984836717347,152.56111542586643,201.9823218314288,23.63622915048635,75.20618366063839,25.78497725507602,268.59351307370855],\"spades\":152.56111542586643,\"targetGod\":176.19734457635278,\"tfLevel\":[133.22238248455943,152.56111542586643,148.2636192166871],\"totalBooks\":128.9248862753801,\"totalGods\":171.89984836717346,\"totalPower\":100.9911609157144,\"winPoints\":275.0397573874775,\"winPointsIncome\":118.18114575243176}"),
        ScoreWeights{ // round 6 (End of the game)
            .winPoints = 1,
        },
    };

    // AllScoreWeights curBestWeights = allScoreWeights;
    // double lr = 2.0;
    // std::default_random_engine rng{1};

    // for (int iter = 0; iter < 10; iter++) {
    //     std::cout << "============================" << std::endl;
    //     std::cout << "iter: " << iter << std::endl;
    //     for (int i = 0; i < 6; i++) {
    //         curBestWeights.at(i).initRandomly(rng);
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
    //                 const auto result = Tournament::playAllInAll({ &bot2, &bot3}, 200);
    //                 const auto skills = GameResult::trueSkill(result);
    //                 if (skills[0] > skills[1] + 0.2) {
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
    //             const auto result = Tournament::playAllInAll({ &bot2, &bot3}, 200);
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

    //     std::cout << "Normalize..." << std::endl;
    //     {
    //         double* weightsPtr = (double*) &curBestWeights;
    //         double s = 0.0;
    //         for (int pos = 0; pos < sizeof(AllScoreWeights) / 8; pos++) {
    //             s += weightsPtr[pos];
    //         }
    //         s /= sizeof(AllScoreWeights) / 8;
    //         for (int pos = 0; pos < sizeof(AllScoreWeights) / 8; pos++) {
    //             weightsPtr[pos] /= s;
    //         }
    //     }
    //     for (int i = 0; i < 6; i++) {
    //         std::cout << "round " << i << ": " << toJson(curBestWeights.at(i)).dump() << std::endl;
    //     }
    // }

    // return 0;

    auto webBot = WebUiBot(std::default_random_engine{1});
    auto cBot = MctsBot(new GreedyBot(allScoreWeights), allScoreWeights, 1000, 2, 4);
    Tournament::playSingleGame({ &webBot, &cBot }, 42);
    return 0;
}
