#include "GameEngine.h"

#include "serialize.h"
#include "StaticData.h"
#include "Utils.h"

#include <array>
#include <iostream>
#include <map>
#include <queue>

constexpr int spadesNeeded__(TerrainType src, TerrainType dst) {
    int dist = (SC(dst) + 7 - SC(src)) % 7;
    dist = std::min(dist, 7 - dist);
    return dist;
}

constexpr std::array<std::array<int, 7>, 7> spadesNeeded_() {
    std::array<std::array<int, 7>, 7> ret;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            ret[i][j] = spadesNeeded__((TerrainType) i, (TerrainType) j);
        }
    }
    return ret;
}

int spadesNeeded(TerrainType src, TerrainType dst) {
    constexpr auto r = spadesNeeded_();
    return r[SC(src)][SC(dst)];
}

GameEngine::GameEngine(std::vector<IBot*> bots, bool withLogs, bool withStats)
    : bots_(std::move(bots))
    , withLogs_(withLogs)
    , withStats_(withStats)
{
}

void GameEngine::doFreeActionMarket(FreeActionMarketType action, GameState& gs) const {
    auto& ps = gs.players[gs.activePlayer];

    switch (action)
    {
        case FreeActionMarketType::ManaToBook: {
            awardResources(IncomableResources{ .anyBook = 1 }, gs);
            spendResources(IncomableResources{ .manaCharge = 5 }, gs);
            break;
        }
        case FreeActionMarketType::BookToGold: {
            ps.resources.gold++;
            spendResources(IncomableResources{ .anyBook = 1 }, gs);
            break;
        }        
        case FreeActionMarketType::ManaToCube: {
            ps.resources.cube++;
            spendResources(IncomableResources{ .manaCharge = 3 }, gs);
            break;
        }
        case FreeActionMarketType::ManaToGold: {
            ps.resources.gold++;
            spendResources(IncomableResources{ .manaCharge = 1 }, gs);
            break;
        }
        case FreeActionMarketType::ManaToHuman: {
            assert (ps.humansLeft > 0);
            awardResources(Resources{ .humans = 1 }, gs);
            spendResources(IncomableResources{ .manaCharge = 5 }, gs);
            break;
        }
        case FreeActionMarketType::HumanToCube: {
            spendResources(Resources{ .humans = 1 }, gs);
            awardResources(Resources{ .cube = 1 }, gs);
            break;
        }
        case FreeActionMarketType::CubeToGold: {
            ps.resources.gold++;
            ps.resources.cube--;
            break;
        }
        case FreeActionMarketType::BurnMana: {
            assert(ps.mana[1] >= 2);
            ps.mana[1] -= 2;
            ps.mana[2]++;
            break;
        }
        default:
            assert(false);
    }
}

void GameEngine::awardBooster(int boosterIdx, GameState& gs) const {
    auto& ps = gs.players[gs.activePlayer];

    const auto newBooster = gs.boosters.at(boosterIdx);
    awardResources(IncomableResources{ .gold = (int8_t) newBooster.gold }, gs);

    if (ps.currentRoundBoosterOriginIdx >= 0) {
        const auto oldBooster = StaticData::roundBoosters()[ps.currentRoundBoosterOriginIdx];
        ps.wpPerEvent[oldBooster.trigger] -= oldBooster.wpPerTrigger;

        gs.boosters.at(boosterIdx) = RoundBoosterOnBoard{
            .originIdx = ps.currentRoundBoosterOriginIdx,
            .gold = 0,
        };
    } else {
        std::remove_if(gs.boosters.begin(), gs.boosters.end(), [newBooster](const RoundBoosterOnBoard& op) { return op.originIdx == newBooster.originIdx; });
        gs.boosters.pop_back();
    }

    ps.currentRoundBoosterOriginIdx = newBooster.originIdx;
    const auto& origin = StaticData::roundBoosters()[newBooster.originIdx];
    ps.wpPerEvent[origin.trigger] += origin.wpPerTrigger;

    if (origin.buttonOriginIdx >= 0) {
        ps.boosterButton = Button{ .buttonOrigin = origin.buttonOriginIdx, .isUsed = false };
    } else {
        ps.boosterButton = Button{ .buttonOrigin = -1, .isUsed = true };
    }
}

void GameEngine::chargeOpp(int8_t pos, GameState& gs) const {
    int power = gs.cache->fieldByState_[gs.fieldStateIdx].adjacentEnemiesPower(pos, gs.activePlayer);
    if (power > 0) {
        const auto& oppPs = gs.players[1 - gs.activePlayer];
        if (power == 1) {
            gs.activePlayer = 1 - gs.activePlayer;
            charge(1, gs);
            gs.activePlayer = 1 - gs.activePlayer;
        } else {
            auto oppBot = bots_[1 - gs.activePlayer];
            int chargable = oppPs.mana[0] * 2 + oppPs.mana[1];
            power = std::min(power, chargable);
            if (oppBot->wannaCharge(gs, power)) {
                gs.activePlayer = 1 - gs.activePlayer;
                charge(power, gs);
                awardWp(1 - power, gs);
                gs.activePlayer = 1 - gs.activePlayer;
            }
        }
    }
}

void GameEngine::upgradeBuilding(int8_t pos, Building building, GameState& gs, int param) const {
    auto& ps = getPs(gs);
    const auto& bot = bots_[gs.activePlayer];
    ps.buildingsAvailable[building]--;
    ps.buildingsAvailable[gs.field().building[pos].type]++;
    gs.field().populateField(gs, FieldActionType::ChangeBuildingType, pos, SC(building), 0);
    awardWp(ps.wpPerEvent[StaticData::buildingOrigins()[building].buildEvent], gs);

    if (building == Building::Palace) {
        assert(param >= 0);
        ps.palaceIdx = param;
        std::remove(gs.palacesAvailable.begin(), gs.palacesAvailable.end(), param);
        gs.palacesAvailable.pop_back();

        const auto& pal = StaticData::palaces()[param];
        if (pal.buttonOrigin >= 0) ps.buttons.push_back(Button{ .buttonOrigin=  pal.buttonOrigin, .isUsed = false });
        ps.additionalIncome += pal.income;
        switch (pal.special) {
            case PalaceSpecial::Charge12book2:
                awardResources(IncomableResources { .anyBook = 2, .manaCharge = 12 }, gs);
                break;
            case PalaceSpecial::Fed6nrg:
                break;
            case PalaceSpecial::FlyingMan:
                break;
            case PalaceSpecial::Fed: {
                const auto tile = bot->chooseFedTile(gs);
                awardFedTile(tile, gs);
                break;
            }
            case PalaceSpecial::FreeGuild: {
                const auto poses = someHexes(false, true, gs);
                const auto pos = bot->choosePlaceToBuildForFree(gs, Building::Guild, false, poses);
                if (pos >= 0) {
                    buildForFree(pos, Building::Guild, false, gs);
                }
                break;
            }
            case PalaceSpecial::GetTech:
                awardTechTile(bot->chooseTechTile(gs), gs);
                break;
            case PalaceSpecial::Guild3wp :
                ps.wpPerEvent[EventType::BuildGuild] += 3;
                break;
            case PalaceSpecial::Lab3wp :
                // Only at EoT
                break;
            case PalaceSpecial::Mine2wp :
                ps.wpPerEvent[EventType::BuildMine] += 2;
                break;
            case PalaceSpecial::Nav2 :
                upgradeNav(gs, true);
                upgradeNav(gs, true);
                break;
            case PalaceSpecial::None :
                // assert(false);
                break;
            case PalaceSpecial::Spades2Books2Bridges2 :
                awardResources(IncomableResources{ .anyBook = 2, .spades = 2, }, gs);
                buildBridge(gs);
                buildBridge(gs);
                break;
            case PalaceSpecial::Wp10 :
                awardWp(10, gs);
                break;
            default:
                assert(false);
                break;
        }
    }

    spendResources(StaticData::buildingOrigins()[building].price, gs);
    constexpr int additionalCharges[] = { 1, 1, 0, 0, 0 };
    if (building == Building::Guild) {
        if (!gs.cache->fieldByState_[gs.fieldStateIdx].hasAdjacentEnemies(pos, gs.activePlayer)) {
            spendResources( Resources{ .gold = 3 }, gs);
        }
        if (ps.buildingsAvailable[Building::Mine] != 5) {
            ps.additionalIncome -= StaticData::buildingOrigins()[Building::Mine].income;
        }
        if (getColor(gs) == TerrainType::Mountain && ps.buildingsAvailable[Building::Guild] == 3) {
            ps.additionalIncome.gold++;
        }
        ps.additionalIncome.manaCharge += additionalCharges[ps.buildingsAvailable[Building::Guild]];
    }

    if (building == Building::Laboratory || building == Building::Academy) {
        awardTechTile((TechTile) param, gs);
    }

    if (building != Building::Academy) {
        // Academy doesn't actually give us additional income
        ps.additionalIncome += StaticData::buildingOrigins()[building].income;
    }

    if (building == Building::Laboratory || building == Building::Palace) {
        ps.additionalIncome -= StaticData::buildingOrigins()[Building::Guild].income;
        ps.additionalIncome.manaCharge -= additionalCharges[ps.buildingsAvailable[Building::Guild] - 1];
        if (getColor(gs) == TerrainType::Mountain && ps.buildingsAvailable[Building::Guild] == 4) {
            ps.additionalIncome.gold--;
        }
    }

    chargeOpp(pos, gs);

    checkFederation(gs);
}

void GameEngine::awardTechTile(TechTile tile, GameState& gs) const {
    auto& ps = gs.players[gs.activePlayer];
    ps.techTiles[tile] = true;

    switch (tile) {
        case TechTile::p3g2: {
            ps.additionalIncome.gold += 2;
            ps.additionalIncome.winPoints += 3;
            break;
        }
        case TechTile::BookCharge: {
            ps.additionalIncome.anyBook++;
            ps.additionalIncome.manaCharge++;
            break;
        }
        
        case TechTile::cubeGod: {
            ps.additionalIncome.cube += 1;
            ps.additionalIncome.anyGod += 1;
            break;
        }
        case TechTile::charge4: {
            ps.buttons.push_back( Button { .buttonOrigin = 3, .isUsed = false} );
            break;
        }
        case TechTile::annex: {
            ps.annexLeft = 2;
            break;
        }
        case TechTile::cube5p2g: {
            awardResources(Resources{ .gold = 2, .cube = 1, .winPoints = 5 }, gs);
            break;
        }
        case TechTile::putGod2p: {
            ps.wpPerEvent[EventType::PutManToGod] += 2;
            break;
        }
        case TechTile::scoreEdge : {
            ps.wpPerEvent[EventType::BuildOnEdge] += 3;
            break;
        }
        case TechTile::scoreFeds : {
            break;
        }
        case TechTile::scoreMinGod : {
            break;
        }
        case TechTile::spades2 : {
            awardResources(IncomableResources{ .spades = 2 }, gs);
            break;
        }
        case TechTile::tower : {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bots_[gs.activePlayer]->choosePlaceToBuildForFree(gs, Building::Tower, true, poses);
            if (pos >= 0) {
                buildForFree(pos, Building::Tower, true, gs);
            }
            break;
        }
        default:
            assert(false);
    }

    awardResources(gs.staticGs.bookAndGodPerTech[tile], gs);
    if (getRace(gs) == Race::Philosophers) {
        FlatMap<BookColor, int8_t, 4> books;
        for (const auto [color, val] : gs.staticGs.bookAndGodPerTech[tile].gods) {
            if (val > 0) {
                books[color] = 1;
                break;
            }
        }
        awardResources(Resources{ .books = books }, gs);
    }
}

InnoPrice GameEngine::getInnoFullPrice(int pos, const GameState& gs) const {
    InnoPrice price = StaticData::innoPrices()[pos];
    const auto& ps = gs.players[gs.activePlayer];
    if (ps.buildingsAvailable[Building::Palace] > 0) price.gold += 5;
    price.anyBooks += ps.innovations.size();
    if (getColor(gs) == TerrainType::Wasteland && ps.innovations.size() == 1) price.anyBooks--;

    return price;
}

// **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  
//   --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
// **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  
//   --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
// **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  
//   --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
// **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  **  
//   --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --

std::vector<Action> GameEngine::generateActions(const GameState& gs) const {
    std::vector<Action> ret;
    const auto& ps = gs.players[gs.activePlayer];
    const auto color = gs.staticGs.playerColors[gs.activePlayer];
    const auto race = gs.staticGs.playerRaces[gs.activePlayer];

    if (ps.passed) {
        return ret;
    }

    // UpgradeBuilding,
    if (ps.buildingsAvailable[Building::Guild] > 0 && ps.resources >= StaticData::buildingOrigins()[Building::Guild].price) {
        for (const auto pos: gs.cache->fieldByState_[gs.fieldStateIdx].buildingByPlayer(Building::Mine, gs.activePlayer)) {
            int extraPrice = 0;
            if (!gs.cache->fieldByState_[gs.fieldStateIdx].hasAdjacentEnemies(pos, gs.activePlayer)) {
                extraPrice = 3;
            }
            if (ps.resources.gold >= 3 + extraPrice) {
                ret.emplace_back(Action{
                    .type = ActionType::UpgradeBuilding,
                    .param1 = pos,
                    .param2 = extraPrice,
                });
            }
        }
    }
    if (ps.buildingsAvailable[Building::Palace] > 0 && (ps.resources >= StaticData::buildingOrigins()[Building::Palace].price)) {
        for (const auto pos: gs.cache->fieldByState_[gs.fieldStateIdx].buildingByPlayer(Building::Guild, gs.activePlayer)) {
            for (const auto& palaceIdx: gs.palacesAvailable) {
                ret.emplace_back(Action{
                    .type = ActionType::UpgradeBuilding,
                    .param1 = pos,
                    .param2 = palaceIdx,
                });
            }
        }
    }
    if (ps.buildingsAvailable[Building::Laboratory] > 0 && (ps.resources >= StaticData::buildingOrigins()[Building::Laboratory].price)) {
        for (int i = 0; i < 12; i++) {
            if (!ps.techTiles[(TechTile) i]) {
                for (const auto pos: gs.cache->fieldByState_[gs.fieldStateIdx].buildingByPlayer(Building::Guild, gs.activePlayer)) {
                    ret.emplace_back(Action{
                        .type = ActionType::UpgradeBuilding,
                        .param1 = pos,
                        .param2 = -1 - i,
                    });
                }
            }
        }
    }
    if (ps.buildingsAvailable[Building::Academy] > 0 && (ps.resources >= StaticData::buildingOrigins()[Building::Academy].price)) {
        for (int i = 0; i < 12; i++) {
            if (!ps.techTiles[(TechTile) i]) {
                for (const auto pos: gs.cache->fieldByState_[gs.fieldStateIdx].buildingByPlayer(Building::Laboratory, gs.activePlayer)) {
                    ret.emplace_back(Action{
                        .type = ActionType::UpgradeBuilding,
                        .param1 = pos,
                        .param2 = i,
                    });
                }
            }
        }
    }

    // Market,

    int8_t manaDiscount = (race == Race::Illusionists) ? 1 : 0;
    if (ps.bridgesLeft > 0 && !gs.marketActions[0].isUsed && ps.mana[2] >= 3 - manaDiscount) {
        for (const auto idxFrom: gs.cache->fieldByState_[gs.fieldStateIdx].ownedByPlayer.at(gs.activePlayer)) {
            for (const auto bridge: StaticData::fieldOrigin().bridgeIds.at(idxFrom)) {
                if (gs.cache->fieldByState_[gs.fieldStateIdx].bridges.at(bridge) == -1) {
                    ret.emplace_back(Action{
                        .type = ActionType::Market,
                        .param1 = 0,
                        .param2 = bridge,
                    });
                }
            }
        }
    }

    if (ps.humansLeft > 0 && !gs.marketActions[1].isUsed && ps.mana[2] >= 3 - manaDiscount) {
        ret.emplace_back(Action{
            .type = ActionType::Market,
            .param1 = 1,
        });
    }

    for (int ma = 2; ma < 6; ma++) {
        if (!gs.marketActions[ma].isUsed && ps.mana[2] >= gs.marketActions[ma].manaPrice - manaDiscount) {
            ret.emplace_back(Action{
                .type = ActionType::Market,
                .param1 = ma,
            });
        }
    }

    const auto totalBooks = sum(ps.resources.books.values());
    for (const auto [idx, action]: enumerate(gs.bookActions)) {
        if (!action.isUsed && totalBooks >= action.bookPrice) {
            const auto& origin = StaticData::buttonOrigins()[action.buttonOrigin];
            if (origin.special == ButtonActionSpecial::UpgradeMine) {
                for (const auto minePos: gs.cache->fieldByState_[gs.fieldStateIdx].buildingByPlayer(Building::Mine, gs.activePlayer)) {
                    ret.emplace_back(Action{
                        .type = ActionType::BookMarket,
                        .param1 = (int) idx,
                        .param2 = minePos,
                    }); 
                }
            } else {
                ret.emplace_back(Action{
                    .type = ActionType::BookMarket,
                    .param1 = (int) idx,
                });
            }
        }
    }

    // ActivateAbility,
    // booster action
    if (!ps.boosterButton.isUsed && ps.boosterButton.buttonOrigin >= 0) {
        const auto& origin = StaticData::buttonOrigins().at(ps.boosterButton.buttonOrigin);
        if (origin.special == ButtonActionSpecial::None) {
            ret.emplace_back(Action{
                .type = ActionType::ActivateAbility,
                .param1 = -1,
            });
        } else if (origin.special == ButtonActionSpecial::BuildBridge) {
            if (ps.bridgesLeft > 0) {
                for (const auto idxFrom: gs.cache->fieldByState_[gs.fieldStateIdx].ownedByPlayer.at(gs.activePlayer)) {
                    for (const auto bridge: StaticData::fieldOrigin().bridgeIds.at(idxFrom)) {
                        if (gs.cache->fieldByState_[gs.fieldStateIdx].bridges.at(bridge) == -1) {
                            ret.emplace_back(Action{
                                .type = ActionType::ActivateAbility,
                                .param1 = -1,
                                .param2 = bridge,
                            });
                        }
                    }
                }
            }
        } else {
            assert(false); // Other variants are invalid
        }
    }

    for (const auto [idx, button]: enumerate(ps.buttons)) {
        if (!button.isUsed) {
            const auto& origin = StaticData::buttonOrigins().at(button.buttonOrigin);
            switch (origin.special) {
                case ButtonActionSpecial::None: {
                    ret.emplace_back(Action{
                        .type = ActionType::ActivateAbility,
                        .param1 = (int) idx,
                    });
                    break;
                }
                case ButtonActionSpecial::BuildBridge: {
                    assert(false); // Invalid button
                    break;
                }
                case ButtonActionSpecial::FiraksButton: {
                    if (ps.buildingsAvailable[Building::Guild] > 0) {
                        for (const auto labPos: gs.cache->fieldByState_[gs.fieldStateIdx].buildingByPlayer(Building::Laboratory, gs.activePlayer)) {
                            ret.emplace_back(Action{
                                .type = ActionType::ActivateAbility,
                                .param1 = (int) idx,
                                .param2 = labPos,
                            });
                        }
                    }
                    break;
                }
                case ButtonActionSpecial::WpForGuilds2: {
                    assert(false); // Invalid button
                }
                case ButtonActionSpecial::UpgradeMine: {
                    for (const auto minePos: gs.cache->fieldByState_[gs.fieldStateIdx].buildingByPlayer(Building::Mine, gs.activePlayer)) {
                        ret.emplace_back(Action{
                            .type = ActionType::ActivateAbility,
                            .param1 = (int) idx,
                            .param2 = minePos,
                        }); 
                    }
                    break;
                }
                default:
                    assert(false);
            }
        }
    }

    // PutManToGod,
    if (ps.resources.humans > 0) {
        for (int i = 0; i < 4; i++) {
            const auto gc = (GodColor) i;
            if (gs.humansOnGods[gc].size() < 3) {
                ret.emplace_back(Action{
                    .type = ActionType::PutManToGod,
                    .param1 = i,
                    .param2 = 0,
                });
            }
            ret.emplace_back(Action{
                .type = ActionType::PutManToGod,
                .param1 = i,
                .param2 = 1,
            });
        }
    }

    // GetInnovation,
    if (ps.innovations.size() < 3) {
        for(const auto [idx, inno]: enumerate(gs.innovations)) {
            if (inno != Innovation::None) {
                const auto price = getInnoFullPrice(idx, gs);
                if (ps.resources >= price) {
                    ret.emplace_back(Action{
                        .type = ActionType::GetInnovation,
                        .param1 = (int) idx,
                    });
                }
            }
        }
    }

    // TerraformAndBuild,
    // No "pure" terraforms, only with build
    if (ps.buildingsAvailable[Building::Mine] > 0 && ps.resources >= StaticData::buildingOrigins()[Building::Mine].price) {
        const auto poses = someHexes(true, false, gs, 1, 0);
        for (const auto& pos: poses) {
            if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::None) {
                ret.emplace_back(Action{
                    .type = ActionType::TerraformAndBuild,
                    .param1 = pos,
                    .param2 = -1,
                });
            }
        }
    }

    if (ps.navLevel < 3 && ps.resources >= Resources { .gold = 4, .humans = 1 }) {
        ret.emplace_back(Action{
            .type = ActionType::UpgradeNav
        });
    }
    int8_t goldDiscount = (color == TerrainType::Plains)? 4 : 0;
    if (ps.tfLevel < 2 && ps.resources >= Resources { .gold = (int8_t) (5 - goldDiscount), .cube = 1, .humans = 1 }) {
        ret.emplace_back(Action{
            .type = ActionType::UpgradeTerraform
        });
    }

    // Annex,
    if (ps.annexLeft > 0) {
        for (const auto& bIdx: gs.cache->fieldByState_[gs.fieldStateIdx].ownedByPlayer[gs.activePlayer]) {
            if (!gs.cache->fieldByState_[gs.fieldStateIdx].building[bIdx].hasAnnex) {
                ret.emplace_back(Action{
                    .type = ActionType::Annex,
                    .param1 = bIdx
                });
            }
        }
    }

    // Pass, take booster
    for (int i = 0; i < 3; i++) {
        ret.emplace_back(Action{ .type = ActionType::Pass, .param1 = i});
    }

    return ret;
}

void GameEngine::putManToGod(GodColor color, bool discard, GameState& gs) const {
    auto& ps = getPs(gs);
    assert(ps.resources.humans > 0);
    ps.resources.humans--;

    if (discard) {
        ps.humansLeft++;
        moveGod(1, color, gs);
    } else {
        auto& gd = gs.humansOnGods[color];
        assert(gd.size() < 3);
        constexpr int movePerPos[] = {3, 2, 2};
        moveGod(movePerPos[gd.size()], color, gs);
        gd.push_back(gs.activePlayer);
    }

    awardWp(ps.wpPerEvent[EventType::PutManToGod], gs);
}

void GameEngine::buildBridge(GameState& gs) const {
    const auto& ps = gs.players[gs.activePlayer];
    const auto& bot = bots_.at(gs.activePlayer);

    if (getPs(gs).bridgesLeft > 0) {
        const auto poses = gs.cache->fieldByState_[gs.fieldStateIdx].buildableBridges(gs.activePlayer);
        const auto pos = bot->choosePlaceForBridge(gs, poses);
        if (pos >= 0) {
            buildBridge(pos, gs);
        }
    }
}

void GameEngine::buildBridge(int8_t pos, GameState& gs) const {
    gs.field().populateField(gs, FieldActionType::BuildBridge, pos);
    assert(getPs(gs).bridgesLeft > 0);
    getPs(gs).bridgesLeft--;

    checkFederation(gs);
}

void GameEngine::pushButton(int8_t buttonIdx, int param, GameState& gs) const {
    const auto& ps = gs.players[gs.activePlayer];
    const auto& bot = bots_.at(gs.activePlayer);

    const auto& button = StaticData::buttonOrigins()[buttonIdx];
    awardResources(button.resources, gs);

    switch (button.special) {
        case ButtonActionSpecial::BuildBridge: {
            if (param < 0) {
                buildBridge(gs);
            } else {
                buildBridge(param, gs);
            }
            break;
        }
        case ButtonActionSpecial::FiraksButton: {
            if (param < 0) {
                std::vector<int8_t> possiblePos;
                possiblePos.reserve(10);
                for (const auto& pos : gs.cache->fieldByState_[gs.fieldStateIdx].ownedByPlayer[gs.activePlayer]) {
                    if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Laboratory) {
                        possiblePos.push_back(pos);
                    }
                }
                param = bot->chooseBuildingToConvertForFree(gs, Building::Guild, possiblePos);
            }
            upgradeBuilding(param, Building::Guild, gs);
            break;
        }
        case ButtonActionSpecial::UpgradeMine: {
            if (param < 0) {
                std::vector<int8_t> possiblePos;
                possiblePos.reserve(10);
                for (const auto& pos : gs.cache->fieldByState_[gs.fieldStateIdx].ownedByPlayer[gs.activePlayer]) {
                    if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Mine) {
                        possiblePos.push_back(pos);
                    }
                }
                param = bot->chooseBuildingToConvertForFree(gs, Building::Guild, possiblePos);
            }
            upgradeBuilding(param, Building::Guild, gs);
            break;
        }
        case ButtonActionSpecial::WpForGuilds2: {
            awardWp(2 * ps.countBuildings(Building::Guild), gs);
            break;
        }
        case ButtonActionSpecial::None: {
            break;
        }
    };
}

void GameEngine::awardInnovation(Innovation inno, GameState& gs) const {
    auto& ps = gs.players[gs.activePlayer];
    const auto& bot = bots_.at(gs.activePlayer);

    ps.innovations.push_back(inno);
    awardWp(ps.wpPerEvent[EventType::GetInvention], gs);

    switch (inno) {
        case Innovation::AcademyAnd2wp: {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bot->choosePlaceToBuildForFree(gs, Building::Academy, true, poses);
            if (pos >= 0) buildForFree(pos, Building::Academy, true, gs);
            ps.additionalIncome.winPoints += 2;
            break;
        }
        case Innovation::Bridges: {
            awardWp(3 * (3 - ps.bridgesLeft), gs);
            break;
        }
        case Innovation::Feds5wp: {
            awardWp(5 * ps.feds.size(), gs);
            break;
        }
        case Innovation::Gods2xwp: {
            auto vals = ps.resources.gods.values();
            std::sort(vals.rbegin(), vals.rend());
            awardWp(vals[0] + vals[1], gs);
            break;
        }
        case Innovation::GodsAnd10wp: {
            awardWp(10, gs);
            int8_t types = 0;
            for (int i = 0; i < 7; i++) {
                if (ps.countBuildings((Building) i) > 0) types++;
            }
            awardResources(IncomableResources { .anyGod = (int8_t) (-types) }, gs);
            break;
        }
        case Innovation::GroupsWp : {
            const auto nGroups = countGroups(gs);
            if (nGroups >= 6) { 
                awardWp(18, gs);
            } else if (nGroups == 5) { 
                awardWp(12, gs);
            } if (nGroups == 4) { 
                awardWp(8, gs);
            }
            break;
        }
        case Innovation::Guild2Wp : {
            // already implemented
            break;
        }
        case Innovation::GuildAnd5gold : {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bot->choosePlaceToBuildForFree(gs, Building::Guild, true, poses);
            if (pos >= 0) buildForFree(pos, Building::Guild, true, gs);
            ps.additionalIncome.gold += 5;
            break;
        }
        case Innovation::Human3wpButton : {
            ps.buttons.push_back(Button{ .buttonOrigin = 10, .isUsed = false });
            break;
        }
        case Innovation::HumanNavTerraform : {
            awardResources(Resources{ .humans = 1 }, gs);
            upgradeNav(gs, true);
            upgradeTerraform(gs, true);
            break;
        }
        case Innovation::LabAndTech : {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bot->choosePlaceToBuildForFree(gs, Building::Laboratory, true, poses);
            if (pos >= 0) buildForFree(pos, Building::Laboratory, true, gs);
            awardTechTile(bot->chooseTechTile(gs), gs);
            break;
        }
        case Innovation::Labs5wp : {
            awardWp(5 * ps.countBuildings(Building::Laboratory), gs);
            break;
        }
        case Innovation::MineAnd3cubes : {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bot->choosePlaceToBuildForFree(gs, Building::Mine, true, poses);
            if (pos >= 0) buildForFree(pos, Building::Mine, true, gs);
            ps.additionalIncome.cube += 3;
            break;
        }
        case Innovation::MinesWp : {
            awardWp(2 * ps.countBuildings(Building::Mine), gs);
            break;
        }
        case Innovation::MonumentAnd7wp : {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bot->choosePlaceToBuildForFree(gs, Building::Monument, true, poses);
            if (pos >= 0) {
                buildForFree(pos, Building::Monument, true, gs);
            }
            awardWp(7, gs);
            break;
        }
        case Innovation::Nbuildings : {
            const auto b = gs.cache->fieldByState_[gs.fieldStateIdx].ownedByPlayer[gs.activePlayer].size();
            if (b >= 11) {
                awardWp(18, gs);
            } else if (b >= 9) {
                awardWp(12, gs);
            } else if (b >= 7) {
                awardWp(8, gs);
            }
            break;
        }
        case Innovation::None : {
            assert(false);
            break;
        }
        case Innovation::PalaceAndMana : {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bot->choosePlaceToBuildForFree(gs, Building::Palace, true, poses);
            if (pos >= 0) {
                buildForFree(pos, Building::Palace, true, gs);
            }
            ps.mana[2] += 2;
            ps.additionalIncome.manaCharge += 4;
            break;
        }
        case Innovation::SpadeBookGods : {
            ps.buttons.push_back( Button{ .buttonOrigin = 1, .isUsed = false });
            awardResources(IncomableResources{ .anyGod = -20, .anyBook = 1 }, gs);
            break;
        }
        default:
            assert(false);
    }
}

void GameEngine::upgradeNav(GameState& gs, bool forFree) const {
    auto& ps = gs.players[gs.activePlayer];
    assert(ps.navLevel < 3 || forFree);
    if (ps.navLevel < 3) {
        ps.navLevel++;

        if (!forFree) {
            spendResources(Resources{ .gold = 4, .humans = 1 }, gs);
        }

        if (ps.navLevel == 1) {
            awardWp(2, gs);
        } else if (ps.navLevel == 2) {
            if (getColor(gs) == TerrainType::Lake) {
                awardWp(3, gs);
            } else {
                awardResources( IncomableResources{ .anyBook = 2 }, gs);
            }
        } else if (ps.navLevel == 3) {
            if (getColor(gs) == TerrainType::Lake) {
                awardResources( IncomableResources{ .anyBook = 2 }, gs);
            } else {
                awardWp(4, gs);
            }
        }
    }
}

void GameEngine::upgradeTerraform(GameState& gs, bool forFree) const {
    auto& ps = gs.players[gs.activePlayer];
    assert(ps.tfLevel < 2 || forFree);
    if (ps.tfLevel < 2) {
        ps.tfLevel++;
        if (!forFree) {
            spendResources(Resources{ .gold = 5, .cube = 1, .humans = 1 }, gs);
            if (getColor(gs) == TerrainType::Plains) {
                ps.resources.gold += 4;
            }
        }

        if (ps.tfLevel == 1) {
            awardResources( IncomableResources{ .anyBook = 2 }, gs);
        } else if (ps.tfLevel == 2) {
            awardWp(6, gs);
        }
    }
}

void GameEngine::doTurnGuided(GameState& gs) const {
    if (getPs(gs).passed) {
        return;
    }

    const auto bot = bots_.at(gs.activePlayer);
    auto actions = generateActions(gs);
    auto fullAction = bot->chooseAction(gs, actions);
    
    while (fullAction.action.type == ActionType::None) {
        for (const auto& a: fullAction.preAction) {
            doFreeActionMarket(a, gs);
        }
        actions = generateActions(gs);
        fullAction = bot->chooseAction(gs, actions);
    }

    for (const auto& a: fullAction.preAction) {
        doFreeActionMarket(a, gs);
    }
    doAction(fullAction.action, gs);
    for (const auto& a: fullAction.postAction) {
        doFreeActionMarket(a, gs);
    }
}

void GameEngine::buildForFree(int8_t pos, Building building, bool isNeutral, GameState& gs) const {
    assert(pos >= 0 && pos < FieldOrigin::FIELD_SIZE);
    assert(gs.field().building[pos].type == Building::None);
    auto& ps = getPs(gs);

    if (gs.field().type[pos] != getColor(gs)) {
        int amount = spadesNeeded(gs.cache->fieldByState_[gs.fieldStateIdx].type[pos], getColor(gs));
        constexpr int cubePerTf[] = { 3, 2, 1 };
        spendResources(Resources { .cube = (int8_t) (cubePerTf[ps.tfLevel] * amount) }, gs);
        terraform(pos, amount, gs);
    }
    gs.field().populateField(gs, FieldActionType::BuildNew, pos, SC(building), isNeutral);

    if (!isNeutral) {
        ps.additionalIncome += StaticData::buildingOrigins()[building].income;
        ps.buildingsAvailable[building]--;

        if (building == Building::Mine && ps.buildingsAvailable[Building::Mine] == 4) ps.additionalIncome.cube--;
        
        constexpr int additionalCharges[] = { 1, 1, 0, 0, 0 };
        if (building == Building::Guild) ps.additionalIncome.manaCharge += additionalCharges[ps.buildingsAvailable[Building::Guild]];
    } else {
        ps.neutralBuildingsAmount[building]++;
        if (building == Building::Tower) ps.additionalIncome += StaticData::buildingOrigins()[building].income;
    }

    awardWp(ps.wpPerEvent[StaticData::buildingOrigins()[building].buildEvent], gs);

    if (!(gs.round == 0 && gs.phase == GamePhase::Upkeep)) {
        chargeOpp(pos, gs);
        checkFederation(gs);
    }
}

void GameEngine::buildMine(int8_t pos, GameState& gs) const {
    auto& ps = getPs(gs);

    assert(gs.field().building[pos].type == Building::None);
    assert(ps.buildingsAvailable[Building::Mine] > 0);
    gs.field().populateField(gs, FieldActionType::BuildNew, pos, SC(Building::Mine), 0);

    if (ps.buildingsAvailable[Building::Mine] != 5) ps.additionalIncome.cube++;
    ps.buildingsAvailable[Building::Mine]--;

    spendResources(StaticData::buildingOrigins()[Building::Mine].price, gs);

    awardWp(ps.wpPerEvent[EventType::BuildMine], gs);
    if (StaticData::fieldOrigin().onEdge_[pos]) {
        awardWp(ps.wpPerEvent[EventType::BuildOnEdge], gs);
    }
    if (StaticData::fieldOrigin().isNearRiver[pos]) {
        awardWp(ps.wpPerEvent[EventType::BuildNearRiver], gs);
    }

    chargeOpp(pos, gs);
    checkFederation(gs);
}

void GameEngine::terraformAndBuildMine(int8_t pos, bool build, GameState& gs) const {
    const auto amnt = spadesNeeded(gs.cache->fieldByState_[gs.fieldStateIdx].type[pos], getColor(gs));
    if (amnt > 0) {
        constexpr int8_t cubePerTf[] = { 3, 2, 1 };
        spendResources(Resources{ .cube = (int8_t) (amnt * cubePerTf[getPs(gs).tfLevel]) }, gs);
        terraform(pos, amnt, gs);
    }
    if (build) buildMine(pos, gs);
}

// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------

void GameEngine::doAction(Action action, GameState& gs) const {
    auto& ps = gs.players[gs.activePlayer];
    const auto& bot = bots_.at(gs.activePlayer);
    const auto race = getRace(gs);

    switch (action.type) {
        case ActionType::UpgradeBuilding: {
            const auto pos = action.param1;
            if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Guild) {
                if (action.param2 >= 0) {
                    upgradeBuilding(pos, Building::Palace, gs, action.param2);
                } else {
                    upgradeBuilding(pos, Building::Laboratory, gs, -1 - action.param2);
                }
            } else if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Mine) {
                upgradeBuilding(pos, Building::Guild, gs, action.param2);
            } else if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Laboratory) {
                upgradeBuilding(pos, Building::Academy, gs, action.param2);
            }
            break;
        }
        case ActionType::TerraformAndBuild: {
            terraformAndBuildMine(action.param1, action.param2, gs);
            break;
        }
        case ActionType::UpgradeNav: {
            upgradeNav(gs);
            break;
        }
        case ActionType::UpgradeTerraform: {
            upgradeTerraform(gs);
            break;
        }
        case ActionType::GetInnovation: {
            const auto price = getInnoFullPrice(action.param1, gs);
            spendResources(Resources { .gold = price.gold, .books = price.books }, gs);
            spendResources(IncomableResources { .anyBook = price.anyBooks }, gs);

            awardInnovation(gs.innovations[action.param1], gs);
            gs.innovations[action.param1] = Innovation::None;
            break;
        }
        case ActionType::PutManToGod: {
            putManToGod((GodColor) action.param1, action.param2, gs);
            break;
        }
        case ActionType::BookMarket: {
            auto& bookAction = gs.bookActions[action.param1];
            spendResources(IncomableResources { .anyBook = bookAction.bookPrice }, gs);
            assert(bookAction.isUsed == 0);
            bookAction.isUsed = true;
            if (bookAction.buttonOrigin >= 0) {
                pushButton(bookAction.buttonOrigin, action.param2, gs);
            }
            break;
        }

        case ActionType::ActivateAbility: {
            if (action.param1 < 0) {
                assert(ps.boosterButton.isUsed == 0);
                ps.boosterButton.isUsed = true;
                pushButton(ps.boosterButton.buttonOrigin, action.param2, gs);
            } else {
                auto& aAction = ps.buttons[action.param1];
                assert(aAction.isUsed == 0);
                aAction.isUsed = true;
                pushButton(aAction.buttonOrigin, action.param2, gs);
            }
            break;
        }

        case ActionType::Market: {
            auto& marketAction = gs.marketActions[action.param1];
            int8_t manaDiscount = (race == Race::Illusionists) ? 1 : 0;
            spendResources(IncomableResources { .manaCharge = (int8_t) (marketAction.manaPrice - manaDiscount) }, gs);
            if (race == Race::Illusionists) {
                awardResources(Resources{ .winPoints = 1}, gs);
            }
            assert(marketAction.isUsed == 0);
            marketAction.isUsed = true;
            if (marketAction.buttonOrigin >= 0) {
                pushButton(marketAction.buttonOrigin, action.param2, gs);
            }
            break;
        }

        case ActionType::Annex: {
            gs.field().populateField(gs, FieldActionType::AddAnnex, action.param1);
            assert(ps.annexLeft > 0);
            ps.annexLeft--;
            checkFederation(gs);
            break;
        }

        case ActionType::Pass: {
            ps.passed = true;
            gs.playersOrder.push_back(gs.activePlayer);

            for (const auto& inno: ps.innovations) {
                if (inno == Innovation::Guild2Wp) {
                    awardWp( 2 * ps.countBuildings(Building::Guild), gs);
                }
            }

            if (ps.palaceIdx >= 0 && (StaticData::palaces()[ps.palaceIdx].special == PalaceSpecial::Lab3wp)) {
                awardWp( 3 * ps.countBuildings(Building::Laboratory), gs);
            }
        
            if (ps.currentRoundBoosterOriginIdx >= 0) {
                if (StaticData::roundBoosters()[ps.currentRoundBoosterOriginIdx].godsForLabs == true) {
                    awardResources(IncomableResources{ .anyGod = ps.countBuildings(Building::Laboratory) }, gs);
                }
                if (StaticData::roundBoosters()[ps.currentRoundBoosterOriginIdx].scoreHuge == true) {
                    awardWp(4 * (ps.countBuildings(Building::Academy) + ps.countBuildings(Building::Palace)), gs);
                }
            }

            if (ps.techTiles[TechTile::scoreFeds]) {
                awardWp( 2 * ps.feds.size(), gs);
            }
            if (ps.techTiles[TechTile::scoreMinGod]) {
                awardWp(minimum(ps.resources.gods.values()), gs);
            }

            awardBooster(action.param1, gs);

            break;
        }
        default:
            assert(false);
    }
}

bool GameEngine::gameEnded(const GameState& gs) const {
    return gs.round >= 6;
}

void GameEngine::doAfterTurnActions(GameState& gs) const {
    if (!gameEnded(gs)) {
        if (gs.phase == GamePhase::Actions) {
            bool allPassed = true;
            for (int i = 0; i < 2; i++) {
                if (!gs.players[i].passed) {
                    allPassed = false;
                }
            }
            if (!allPassed) {
                ++gs.activePlayer;
                gs.activePlayer %= 2;

                while (gs.players[gs.activePlayer].passed) {
                    ++gs.activePlayer;
                    gs.activePlayer %= 2;
                }
            } else {
                gs.phase = GamePhase::EndOfTurn;
            }
        }

        if (gs.phase == GamePhase::EndOfTurn) {
            if (gs.round < 5) {
                const auto& curBonus = StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound[gs.round]];
                for (gs.activePlayer = 0; gs.activePlayer < 2; gs.activePlayer++) {
                    auto& ps = gs.players[gs.activePlayer];
                    const int8_t blessedBonus = (gs.staticGs.playerRaces[gs.activePlayer] == Race::Blessed) ? 3 : 0;
                    for (int i = 0; i < (ps.resources.gods[curBonus.god] + blessedBonus) / curBonus.godAmount; i++) {
                        awardResources(curBonus.resourceBonus, gs);
                    }

                    for (auto& b: ps.buttons) {
                        b.isUsed = false;
                    }

                    ps.wpPerEvent[curBonus.event] -= curBonus.bonusWp;
                    ps.passed = false;
                }

                for (auto& ma: gs.marketActions) {
                    ma.isUsed = false;
                }
                for (auto& ba: gs.bookActions) {
                    ba.isUsed = false;
                }

                for (auto& rb: gs.boosters) {
                    rb.gold++;
                }
            }
            
            gs.round++;
            gs.phase = GamePhase::Upkeep;

            if (gameEnded(gs)) {
                for (gs.activePlayer = 0; gs.activePlayer < 2; gs.activePlayer++) {
                    doFinalScoring(gs);
                }

                for (gs.activePlayer = 0; gs.activePlayer < 2; gs.activePlayer++) {
                    bots_[gs.activePlayer]->triggerFinal(gs);
                }
            }
        }

        dealWithUpkeep(gs);
    }
}

void GameEngine::dealWithUpkeep(GameState& gs) const {
    if (gs.phase == GamePhase::Upkeep && !gameEnded(gs)) {
        const auto& curBonus = StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound[gs.round]];
        const auto& lastRoundBonus = StaticData::roundScoreBonuses()[gs.staticGs.lastRoundBonus];

        for (gs.activePlayer = 0; gs.activePlayer < 2; gs.activePlayer++) {
            auto& ps = gs.players[gs.activePlayer];

            ps.wpPerEvent[curBonus.event] += curBonus.bonusWp;
            if (gs.round == 5) {
                ps.wpPerEvent[lastRoundBonus.event] += lastRoundBonus.bonusWp;
            }
            awardResources(ps.additionalIncome, gs);
            if (ps.currentRoundBoosterOriginIdx >= 0) {
                awardResources(StaticData::roundBoosters()[ps.currentRoundBoosterOriginIdx].resources, gs);
            }
        }

        gs.phase = GamePhase::Actions;
        gs.activePlayer = gs.playersOrder.front();
        gs.playersOrder.clear();
    }
}

void GameEngine::advanceGs(GameState& gs) const {
    if (!gameEnded(gs)) {
        dealWithUpkeep(gs);
        doTurnGuided(gs);
        doAfterTurnActions(gs);
    }
}

void GameEngine::playGame(GameState& gs) const {
    while (!gameEnded(gs)) {
        const auto field = gs.field();
        // std::cout << "cleared " << gs.cache->fieldByState_.size() << std::endl;
        gs.cache->reset();
        gs.fieldStateIdx = 0;
        gs.cache->fieldByState_.push_back(field);

        advanceGs(gs);
    }
}

int GameEngine::countGroups(GameState& gs) const {
    return maximum(gs.cache->fieldByState_[gs.fieldStateIdx].bfs(gs.activePlayer, 0));
}

void GameEngine::log(const std::string& str) const {
    if (withLogs_) std::cerr << str << std::endl;
}

void GameEngine::checkFederation(GameState& gs) const {
    while (gs.field().fedsCount[gs.activePlayer] > getPs(gs).fedsCount) {
        getPs(gs).fedsCount++;
        const auto tile = bots_[gs.activePlayer]->chooseFedTile(gs);
        awardFedTile(tile, gs);
    }
}

void GameEngine::awardWp(int amount, GameState& gs) const {
    getPs(gs).resources.winPoints += amount;
}

int GameEngine::charge(int amount, GameState& gs) const {
    auto& ps = gs.players.at(gs.activePlayer);

    int charge01 = std::min(amount, (int) ps.mana[0]);
    ps.mana[0] -= charge01;
    ps.mana[1] += charge01;
    amount -= charge01;

    int charge12 = 0;
    if (amount > 0) {
        charge12 = std::min(amount, (int) ps.mana[1]);
        ps.mana[1] -= charge12;
        ps.mana[2] += charge12;
    }

    return charge01 + charge12;
}

int GameEngine::moveGod(int amount, GodColor godColor, GameState& gs) const {
    auto& ps = gs.players.at(gs.activePlayer);
    auto& oppPs = gs.players.at(1 - gs.activePlayer);
    auto& psrg = ps.resources.gods[godColor];

    const size_t i = SC(godColor);
    int godCharges = 0;

    if (psrg < 8 && (psrg + amount >= 8)) {
        int flipIdx = -1;
        for (const auto& [idx, fedTile]: enumerate(ps.feds)) {
            if (!fedTile.flipped) {
                flipIdx = idx;
                break;
            }
        }
        if (flipIdx >= 0) {
            ps.feds[flipIdx].flipped = true;
        } else {
            amount = 7 - psrg;
        }
    }

    if (psrg < 9 && (psrg + amount >= 9)) {
        switch (godColor) {
            case GodColor::Yellow:
                ps.additionalIncome.gold += 3;
                break;
            case GodColor::Blue:
                ps.additionalIncome.manaCharge += 6;
                break;
            case GodColor::Brown:
                ps.additionalIncome.cube += 1;
                break;
            case GodColor::White:
                ps.additionalIncome.winPoints += 3;
                break;
            default:
                assert(false);
        };
    }

    if ((psrg <= 2) && (psrg + amount > 2)) {
        godCharges += 1;
    }
    if ((psrg <= 4) && (psrg + amount > 4)) {
        godCharges += 2;
    }
    if ((psrg <= 6) && (psrg + amount > 6)) {
        godCharges += 2;
    }
    if ((psrg <= 11) && (psrg + amount > 11) && (oppPs.resources.gods[godColor] != 12)) {
        godCharges += 3;
    }

    psrg += amount;
    if (psrg > 11) {
        if (oppPs.resources.gods[godColor] == 12) {
            psrg = 11;
        } else {
            psrg = 12;
        }        
    }

    charge(godCharges, gs);

    awardWp(amount * ps.wpPerEvent[EventType::MoveGod], gs);

    return godCharges;
}

void GameEngine::spendResources(IncomableResources resources, GameState& gs) const {
    auto& ps = gs.players.at(gs.activePlayer);

    ps.resources.gold -= resources.gold;
    ps.resources.cube -= resources.cube;
    ps.resources.humans -= resources.humans;
    ps.humansLeft += resources.humans;
    if (resources.anyBook > 0) {
    const auto books = bots_.at(gs.activePlayer)->chooseBooksToSpend(gs, resources.anyBook);
        for (const auto [color, val]: books) {
            ps.resources.books[color] -= val;
        }
    }

    assert (ps.mana[2] >= resources.manaCharge);
    ps.mana[2] -= resources.manaCharge;
    ps.mana[0] += resources.manaCharge;
}

void GameEngine::spendResources(Resources resources, GameState& gs) const {
    auto& ps = gs.players.at(gs.activePlayer);

    ps.resources.gold -= resources.gold;
    ps.resources.cube -= resources.cube;
    ps.resources.humans -= resources.humans;
    ps.humansLeft += resources.humans;
    
    for (const auto [color, val]: resources.books) {
        ps.resources.books[color] -= val;
    }
}

void GameEngine::awardResources(IncomableResources resources, GameState& gs) const {
    auto& ps = gs.players.at(gs.activePlayer);
    const auto bot = bots_.at(gs.activePlayer);

    if (resources.anyGod != 0) {
        if (resources.anyGod > 0) {
            moveGod(resources.anyGod, bot->chooseGodToMove(gs, resources.anyGod), gs);
        }

        if (resources.anyGod > -10 && resources.anyGod < 0) {
            for (int i = resources.anyGod; i < 0; i++) {
                moveGod(1, bot->chooseGodToMove(gs, resources.anyGod), gs);
            }
        }

        if (resources.anyGod == -20) {
            for (size_t i = 0; i < 4; i++) {
                moveGod(1, (GodColor) i, gs);
            }
        }
    }

    if (resources.anyBook > 0) {
        const auto books = bot->chooseBookColorToGet(gs, resources.anyBook);
        for (const auto [color, val]: books) {
            ps.resources.books[color] += val;
        }
    }

    if (resources.manaCharge > 0) {
        charge(resources.manaCharge, gs);
    }

    ps.resources.cube += resources.cube;
    ps.resources.gold += resources.gold;
    const auto hum = std::min(resources.humans, ps.humansLeft);
    ps.resources.humans += hum;
    ps.humansLeft -= hum;
    assert(ps.humansLeft >= 0);
    ps.resources.winPoints += resources.winPoints;

    if (resources.spades > 0) {
        useSpades(resources.spades, gs);
    }
}

Race GameEngine::getRace(const GameState& gs) const {
    return gs.staticGs.playerRaces[gs.activePlayer];
}

PlayerState& GameEngine::getPs(GameState& gs) const {
    assert (gs.activePlayer < 2);
    return gs.players[gs.activePlayer];
}

TerrainType GameEngine::getColor(const GameState& gs) const {
    return gs.staticGs.playerColors[gs.activePlayer];
}

void GameEngine::terraform(int8_t pos, int amount, GameState& gs) const {
    auto& ps = getPs(gs);

    awardWp(amount * ps.wpPerEvent[EventType::Terraform], gs);

    const int8_t dstColor = (int8_t) getColor(gs);
    const int8_t srcColor = (int8_t) gs.cache->fieldByState_[gs.fieldStateIdx].type.at(pos);
    assert(srcColor != dstColor);

    int8_t direction = dstColor - srcColor;
    direction /= (abs(direction) > 3) ? -abs(direction) : abs(direction);

    if (getRace(gs) == Race::Goblins) awardResources(IncomableResources{ .gold = (int8_t) (2 * amount) }, gs);

    gs.field().populateField(gs, FieldActionType::Terraform, pos, ((srcColor + direction * amount + 7) % 7));
}

void GameEngine::useSpades(int amount, GameState& gs) const {
    auto& ps = gs.players.at(gs.activePlayer);
    const auto pColor = gs.staticGs.playerColors[gs.activePlayer];
    const auto bot = bots_.at(gs.activePlayer);

    int spareSpades = amount;
    // if (getRace(gs) == Race::Goblins) awardResources(IncomableResources { .gold = spareSpades * 2 }, gs);

    // [[maybe_unused]] std::size_t str_hash = std::hash<std::string>{}(toJson(gs).dump());
    // if (str_hash == 3574695110) {
    //     str_hash = str_hash;
    // }

    const auto tfAble = terraformableHexes(gs);
    const auto pos = bot->choosePlaceToSpade(gs, spareSpades, tfAble);

    if (pos < 0) {
        return;
    }
    const auto needed = spadesNeeded(gs.cache->fieldByState_[gs.fieldStateIdx].type.at(pos), pColor);

    if (spareSpades < needed) {
        terraform(pos, spareSpades, gs);
        if (gs.phase == GamePhase::Actions) {
            const auto bricks = bot->chooseBricks(gs, pos);
            if (bricks > 0) {
                spendResources(Resources{ .cube = bricks }, gs);
                terraform(pos, needed - spareSpades, gs);
            }
        }
        spareSpades = 0;
    } else {
        spareSpades -= needed;
        terraform(pos, needed, gs);
    }

    while (spareSpades > 0) {
        const auto pos = bot->choosePlaceToSpade(gs, spareSpades, terraformableHexes(gs));
        if (pos < 0) {
            break;
        }
        const auto needed = spadesNeeded(gs.cache->fieldByState_[gs.fieldStateIdx].type.at(pos), pColor);
        const auto used = std::min(spareSpades, needed);
        terraform(pos, used, gs);
        spareSpades -= used;
    }

    if (gs.phase == GamePhase::Actions && (ps.buildingsAvailable[Building::Mine] > 0) && (gs.cache->fieldByState_[gs.fieldStateIdx].type.at(pos) == pColor) && ps.resources.cube >= 1 && ps.resources.gold >= 2) {
        if (bot->wannaBuildMine(gs, pos)) {
            buildMine(pos, gs);
        }
    }
}

void GameEngine::awardResources(Resources resources, GameState& gs) const {
    auto& ps = gs.players.at(gs.activePlayer);

    for (const auto [color, val]: resources.gods) {
        if (val > 0) {
            moveGod(val, color, gs);
            resources.gods[color] = 0;
        }
    }

    ps.resources += resources;
    ps.humansLeft -= resources.humans;
    if (ps.humansLeft < 0) {
        ps.resources.humans += ps.humansLeft;
        ps.humansLeft = 0;
    }
}

void GameEngine::awardFedTile(FedTileOrigin tile, GameState& gs) const {
    auto& ps = getPs(gs);

    assert(gs.fedTilesAvailable[tile] > 0);
    gs.fedTilesAvailable[tile]--;
    
    ps.feds.push_back(FederationTile{ .origin = tile, .flipped = false });
    awardResources(StaticData::fedTiles()[tile], gs);

    awardWp(ps.wpPerEvent[EventType::FormFederation], gs);
    if (getRace(gs) == Race::Felines) {
        awardResources(IncomableResources{ .anyGod = -3, .anyBook = 1 }, gs);
    } else if (getRace(gs) == Race::Lizards) {
        awardResources(IncomableResources{ .spades = 1 }, gs);
        if (ps.buildingsAvailable[Building::Mine] > 0) {
            const auto poses = someHexes(true, false, gs);
            const auto pos = bots_[gs.activePlayer]->choosePlaceToBuildForFree(gs, Building::Mine, false, poses);
            if (pos != -1) {
                buildForFree(pos, Building::Mine, false, gs);
            }
        }
    }
}

void GameEngine::doFinalScoring(GameState& gs) const {
    auto& ps = getPs(gs);
    auto& oppPs = gs.players[1 - gs.activePlayer];

    const int myCount = gs.field().countReachableBuildings(gs.activePlayer, ps.navLevel);
    const int oppCount = gs.field().countReachableBuildings(1 - gs.activePlayer, oppPs.navLevel);
    const int pNcount = 14;

    int bRank = 0;
    int bTie = 0;
    constexpr int ptsPerBuildingsRank[] = {18, 12, 6, 0};
    if (myCount < oppCount) {
        bRank++;
    } else if (myCount == oppCount) {
        bTie++;
    }
    if (myCount < pNcount) {
        bRank++;
    } else if (myCount == pNcount) {
        bTie++;
    }
    if (bTie == 0) {
        awardWp(ptsPerBuildingsRank[bRank], gs);
    } else if (bTie == 1) {
        awardWp((ptsPerBuildingsRank[bRank] + ptsPerBuildingsRank[bRank + 1]) / 2, gs);
    } else if (bTie == 2) {
        awardWp((ptsPerBuildingsRank[bRank] + ptsPerBuildingsRank[bRank + 1] + ptsPerBuildingsRank[bRank + 2]) / 3, gs);
    }

    for (int i = 0; i < 4; i++) {
        int pos = 0;
        int tie = 0;
        const auto color = (GodColor) i;
        if (ps.resources.gods[color] < gs.staticGs.neutralGods[color]) {
            pos++;
        } else if (ps.resources.gods[color] == gs.staticGs.neutralGods[color]) {
            tie++;
        }
        if (ps.resources.gods[color] < oppPs.resources.gods[color]) {
            pos++;
        } else if (ps.resources.gods[color] == oppPs.resources.gods[color]) {
            tie++;
        }

        constexpr int ptsPerRank[] = {8, 4, 2, 0};
        if (tie == 0) {
            awardWp(ptsPerRank[pos], gs);
        } else if (tie == 1) {
            awardWp((ptsPerRank[pos] + ptsPerRank[pos + 1]) / 2, gs);
        } else if (tie == 2) {
            awardWp((ptsPerRank[pos] + ptsPerRank[pos + 1] + ptsPerRank[pos + 2]) / 3, gs);
        }
    }

    awardWp((ps.resources.cube + sum(ps.resources.books.values()) + ps.resources.gold + ps.resources.humans + ps.mana[1] / 1 + ps.mana[2]) / 5, gs);
}

std::vector<int8_t> GameEngine::terraformableHexes(const GameState& gs) const {
    auto r = someHexes(true, false, gs, 0, 3); // we want all hexes, not only fully terraformable
    r.resize(std::distance(
        r.begin(),
        std::remove_if(r.begin(), r.end(), [t = getColor(gs), &gs](int8_t pos) {
            return gs.cache->fieldByState_[gs.fieldStateIdx].type[pos] == t;
        })
    ));
   
    return r;
}

const std::vector<int8_t>& GameEngine::someHexes(bool onlyInReach, bool onlyNative, const GameState& gs, int cubesDetained, int freeSpades) const {
    const auto& ps = gs.players[gs.activePlayer];

    uint64_t hash = gs.fieldStateIdx;
    hash *= 2;
    hash += onlyInReach ? 1 : 0;
    hash *= 2;
    hash += onlyNative ? 1 : 0;
    hash *= 2;
    hash += gs.activePlayer;
    hash *= 5;
    hash += ps.navLevel;
    const auto cubesLeft = ps.resources.cube - cubesDetained;
    constexpr int8_t tfPrice[] = { 3, 2, 1 };
    int tfLeft = freeSpades;
    if (gs.phase == GamePhase::Actions) {
        tfLeft += cubesLeft / tfPrice[ps.tfLevel];
    }
    tfLeft = std::min(tfLeft, 3);
    hash *= 4;
    hash += tfLeft;

    if (gs.cache->someHexesCache_.contains(hash)) {
        return gs.cache->someHexesCache_.at(hash);
    }

    if (onlyInReach) {
        if (onlyNative) {
            gs.cache->someHexesCache_.emplace(hash, gs.field().reachable(gs.activePlayer, ps.navLevel, getColor(gs)));
            return gs.cache->someHexesCache_.at(hash);
        } else {
            if (tfLeft >= 3) {
                gs.cache->someHexesCache_.emplace(hash, gs.field().reachable(gs.activePlayer, ps.navLevel));
                return gs.cache->someHexesCache_.at(hash);
            } else {
                FlatMap<TerrainType, bool, 7> tfable;
                const auto myColor = getColor(gs);
                for (int i = 0; i < 7; ++i) {
                    if (spadesNeeded(myColor, (TerrainType)i) <= tfLeft) {
                        tfable[(TerrainType) i] = true;
                    } else {
                        tfable[(TerrainType) i] = false;
                    }
                }
                auto poses = gs.cache->fieldByState_[gs.fieldStateIdx].reachable(gs.activePlayer, ps.navLevel);
                poses.resize(std::distance(
                    poses.begin(),
                    std::remove_if(poses.begin(), poses.end(), [&] (int8_t p) {
                        return !tfable[gs.cache->fieldByState_[gs.fieldStateIdx].type[p]];
                    })
                ));

                gs.cache->someHexesCache_.emplace(hash, poses);
                return gs.cache->someHexesCache_.at(hash);
            }
        }
    } else {
        if (onlyNative) {
            std::vector<int8_t> ret;
            ret.reserve(20);
            for (const auto [pos, color]: enumerate(gs.cache->fieldByState_[gs.fieldStateIdx].type)) {
                if (color == getColor(gs) && gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].owner == -1) {
                    ret.emplace_back(pos);
                }
            }

            gs.cache->someHexesCache_.emplace(hash, ret);
            return gs.cache->someHexesCache_.at(hash);
        } else {
            // Why should you want that?
            assert(false);
            return {};
        }
    }
}

void GameEngine::reset() {
}

// --------------------------------------------------------------------------------------------------------------------------------
// ((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))((((((((((((((((
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// --------------------------------------------------------------------------------------------------------------------------------
// ((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))((((((((((((((((
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// --------------------------------------------------------------------------------------------------------------------------------
// ((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))(((((((((((((((())((((()))((((((((((((((((
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void GameEngine::initializeRandomly(GameState& gs, std::default_random_engine& g) const {
    for (size_t i = 0; i < 2; ++i) {
        gs.playersOrder.push_back(i);
    }

    gs.activePlayer = 0;
    gs.round = 0; // pre-game
    
    gs.fedTilesAvailable = {3, 3, 3, 3, 3, 3, 3};

    gs.cache = std::make_shared<PrecalcCache>();
    gs.cache->fieldByState_.reserve(50000);
    gs.cache->someHexesCache_.reserve(50000);
    gs.cache->fieldActionsCache_.reserve(50000);
    gs.fieldStateIdx = Field::newField(*gs.cache).stateIdx;
    
    std::vector<int> indices;

    gs.staticGs.neutralGods[GodColor::Yellow] = 2;
    gs.staticGs.neutralGods[GodColor::Blue] = 2;
    gs.staticGs.neutralGods[GodColor::Brown] = 2;
    gs.staticGs.neutralGods[GodColor::White] = 2;
    indices.resize(12);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 6; ++i) {
        gs.staticGs.bonusByRound.at(i) = indices.at(i);
    }
    int counter = 6;
    while (StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound.at(4)].event == EventType::Terraform) {
        gs.staticGs.bonusByRound.at(4) = indices.at(counter);
        counter++;
    }
    while (StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound.at(5)].event == EventType::Terraform) {
        gs.staticGs.bonusByRound.at(5) = indices.at(counter);
        counter++;
    }
    for (int i = 0; i < 5; ++i) {
        gs.staticGs.neutralGods[StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound.at(i)].god] += StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound.at(i)].godAmount;
    }

    gs.staticGs.lastRoundBonus = 12 + g() % 4;

    const auto bookActions = StaticData::generateBookActions();
    indices.resize(bookActions.size());
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 3; ++i) {
        gs.bookActions.at(i) = bookActions.at(indices.at(i));
    }

    gs.marketActions = StaticData::generateMarketActions();

    indices.resize(12);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 12; ++i) {
        gs.staticGs.techTiles.at(i / 3).at(i % 3) = static_cast<TechTile>(indices.at(i));
        GodColor godColor = (GodColor) (i / 3);
        FlatMap<GodColor, int8_t, 4> gods;
        gods[godColor] = 3 - (i % 3);
        FlatMap<BookColor, int8_t, 4> books;
        books[godColor] = i % 3;

        gs.staticGs.bookAndGodPerTech[(TechTile) indices.at(i)] = Resources{ .gods = gods, .books = books };
    }

    indices.resize(18);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 6; ++i) {
        gs.innovations.at(i) = static_cast<Innovation>(indices.at(i));
    }

    indices.resize(16);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 3; i++) {
        gs.palacesAvailable.push_back(indices.at(i));
    }
    gs.palacesAvailable.push_back(16);


    const auto allRoundBoosters = StaticData::roundBoosters();
    indices.resize(allRoundBoosters.size());
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 5; ++i) {
        gs.boosters.push_back(RoundBoosterOnBoard{ .originIdx = (int8_t) indices.at(i), .gold = 0});
        // gs.staticGs.roundBoosters.at(i) = indices.at(i);
    }

    const auto raceStartBonuses = StaticData::generateRaceStartBonus();
    std::vector<Race> races;
    for (int i = 0; i < 12; i++) {
        races.emplace_back((Race) i);
    };
    for (int i = 0; i < 2; i++) {
        const Race race = bots_[i]->chooseRace(gs, races);
        gs.staticGs.playerRaces[i] = race;
        std::remove(races.begin(), races.end(), race);
        races.pop_back();
        gs.activePlayer = i;
        awardResources(raceStartBonuses[SC(race)].resources, gs);
        awardResources(Resources{.gods = raceStartBonuses[SC(race)].gods}, gs);

        if (race == Race::Navigators) {
            gs.players[i].wpPerEvent[EventType::BuildNearRiver] += 2;
        } else if (race == Race::Psychics) {
            gs.players[i].buttons.push_back(Button{
                .buttonOrigin = 13,
            });
        } else if (race == Race::Philosophers) {
            gs.players[i].buttons.push_back(Button{
                .buttonOrigin = 17,
            });
        }
    }

    const auto landTypeBonuses = StaticData::generateLandTypeBonuses();
    std::vector<TerrainType> colors;
    for (int i = 0; i < 7; i++) {
        colors.emplace_back((TerrainType) i);
    };
    for (int i = 0; i < 2; i++) {
        const TerrainType color = bots_[i]->chooseTerrainType(gs, colors);
        gs.staticGs.playerColors[i] = color;
        std::remove(colors.begin(), colors.end(), color);
        colors.pop_back();
        // gs.activePlayer = i;
        // awardResources(landTypeBonuses[SC(color)].resources, gs);
        // if (color == TerrainType::Lake) {
        //     gs.players[i].navLevel = 1;
        // }
        // if (color == TerrainType::Mountain) {
        //     gs.players[i].additionalIncome.gold += 2;
        // }
    }
    
    gs.activePlayer = 0;
    if (gs.staticGs.playerRaces[0] != Race::Monks) {
        const auto poses = someHexes(false, true, gs);
        auto minePos = bots_[0]->choosePlaceToBuildForFree(gs, Building::Mine, false, poses);
        if (minePos >= 0) {
            buildForFree(minePos, Building::Mine, false, gs);
        }
    }

    gs.activePlayer = 1;
    if (gs.staticGs.playerRaces[1] != Race::Monks) {
        auto poses = someHexes(false, true, gs);
        auto minePos = bots_[1]->choosePlaceToBuildForFree(gs, Building::Mine, false, poses);
        buildForFree(minePos, Building::Mine, false, gs);
        poses = someHexes(false, true, gs);
        minePos = bots_[1]->choosePlaceToBuildForFree(gs, Building::Mine, false, poses);
        buildForFree(minePos, Building::Mine, false, gs);
    }

    gs.activePlayer = 0;
    if (gs.staticGs.playerRaces[0] != Race::Monks) {
        const auto poses = someHexes(false, true, gs);
        auto minePos = bots_[0]->choosePlaceToBuildForFree(gs, Building::Mine, false, poses);
        buildForFree(minePos, Building::Mine, false, gs);
    }

    for (int i = 0; i < 2; ++i) {
        gs.activePlayer = i;

        if (gs.staticGs.playerRaces.at(i) == Race::Monks) {
            const auto poses = someHexes(false, true, gs);
            const auto academyPos = bots_[i]->choosePlaceToBuildForFree(gs, Building::Academy, false, poses);
            buildForFree(academyPos, Building::Academy, false, gs);
        }

        if (gs.staticGs.playerRaces.at(i) == Race::Inventors || gs.staticGs.playerRaces.at(i) == Race::Monks) {
            awardTechTile(bots_[i]->chooseTechTile(gs), gs);
        }
        if (gs.staticGs.playerRaces.at(i) == Race::Omar) {
            const auto poses = someHexes(false, true, gs);
            const auto towerPos = bots_[i]->choosePlaceToBuildForFree(gs, Building::Tower, true, poses);
            buildForFree(towerPos, Building::Tower, true, gs);
        }
    }

    for (int i = 0; i < 2; i++) {
        gs.activePlayer = i;
        const auto color = gs.staticGs.playerColors[i];
        awardResources(landTypeBonuses[SC(color)].resources, gs);
        if (color == TerrainType::Lake) {
            gs.players[i].navLevel = 1;
        }
        if (color == TerrainType::Mountain) {
            gs.players[i].additionalIncome.gold += 2;
        }
        gs.players[i].additionalIncome.cube += 1;
    }

    for (int i = 1; i >= 0; --i) {
        gs.activePlayer = i;
        int idx = bots_[i]->chooseRoundBooster(gs);
        awardBooster(idx, gs);
    }
    for (auto& rb: gs.boosters) {
        rb.gold++;
    }
}
