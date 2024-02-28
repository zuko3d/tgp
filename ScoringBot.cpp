#include "ScoringBot.h"
#include "StaticData.h"
#include "Utils.h"

double ScoringBot::evalPs(const GameState& gs, int pIdx) {
    const auto ap = gs.activePlayer;
    gs.activePlayer = pIdx;
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
        tfs[spadesNeeded(gs.field->type[pos], color)]++;
    }
    for (int i = 0; i < 4; i++) {
        ret += curWeights.reachableHexes[i] * tfs[i];
    }

    ret += curWeights.navLevel[ps.navLevel];
    ret += curWeights.tfLevel[ps.tfLevel];

    if (gs.round < 5) {
        ret += curWeights.targetGod * res.gods[StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound[gs.round]].god];
    }

    return ret;
}
