#include "GameState.h"

#include "GameEngine.h"

GsFeatures GameState::toFeatures(int playerIdx, const GameEngine& ge) const {
    const auto ap = activePlayer;
    GameState* hackedGs = const_cast<GameState*>(this);
    hackedGs->activePlayer = playerIdx;
    const auto& ps = players[playerIdx];

    GsFeatures ret;

    const auto& res = ps.resources;

    ret.gold += res.gold;
    ret.cube += res.cube;
    ret.humans += res.humans;
    ret.totalBooks += sum(res.books.values());
    ret.totalGods += sum(res.gods.values());
    ret.winPoints += res.winPoints;

    ret.goldIncome += ps.additionalIncome.gold;
    ret.cubeIncome += ps.additionalIncome.cube;
    ret.humansIncome += ps.additionalIncome.humans;
    ret.booksIncome += ps.additionalIncome.anyBook;
    ret.godsIncome += ps.additionalIncome.anyGod;
    ret.manaIncome += ps.additionalIncome.manaCharge;

    for (int i = 0; i < 7; i++) {
        const auto cnt = ps.countBuildings((Building) i);
        ret.scorePerBuilding[i] += cnt;
        ret.totalBuildings += cnt;
        ret.totalPower += cnt * StaticData::buildingOrigins()[(Building)i].power;
    }

    if (ps.palaceIdx >= 0) ret.scorePerPalaceIdx[ps.palaceIdx] += 1;
    for (const auto [tile, present]: ps.techTiles) {
        if (present) {
            ret.scorePerTech[SC(tile)] += 1;
        }
    }
    for (const auto inno: ps.innovations) {
        ret.scorePerInnovation[SC(inno)] += 1;
    }
    
    const auto reachableHexes = ge.someHexes(true, false, *this, 0, 0);
    const auto color = staticGs->playerColors[playerIdx];
    std::array<int, 4> tfsNeeded = {{0}};
    for (const auto pos: reachableHexes) {
        tfsNeeded[spadesNeeded(field().type[pos], color)]++;
    }
    for (int i = 0; i < 4; i++) {
        ret.reachableHexes[i] += tfsNeeded[i];
    }

    ret.navLevel[ps.navLevel] += 1;
    ret.tfLevel[ps.tfLevel] += 1;

    if (round < 5) {
        ret.targetGod += res.gods[StaticData::roundScoreBonuses()[staticGs->bonusByRound[round]].god];
    }

    hackedGs->activePlayer = ap;

    return ret;
}
