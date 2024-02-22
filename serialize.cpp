#include "serialize.h"

#include "StaticData.h"

int toJson(int8_t v) {
    return v;
}

nlohmann::json toJson(const Action& op) {
    nlohmann::json j;

    j["type"] = op.type;
    j["param1"] = op.param1;
    j["param2"] = op.param2;

    return j;
}

nlohmann::json toJson(const RoundBoosterOnBoard& op) {
    nlohmann::json j;
    j["originIdx"] = op.originIdx;
    j["gold"] = op.gold;

    return j;
}

nlohmann::json toJson(const RoundScoreBonus& op) {
    nlohmann::json j;

    // EventType event = EventType::None;
    // // int eventParams;
    // int bonusWp;

    // BookColor god;
    // int godAmount;
    // IncomableResources resourceBonus;
    // bool noRound56 = false;

    assert(false);

    return j;
}

nlohmann::json toJson(const IncomableResources& res) {
    nlohmann::json j;

    j["gold"] = res.gold;
    j["cube"] = res.cube;
    j["humans"] = res.humans;
    j["anyGod"] = res.anyGod;
    j["anyBook"] = res.anyBook;
    j["manaCharge"] = res.manaCharge;
    j["spades"] = res.spades;
    j["winPoints"] = res.winPoints;

    return j;
}

nlohmann::json toJson(const Button& res) {
    nlohmann::json j;

    j["buttonOrigin"] = res.buttonOrigin;
    j["isUsed"] = res.isUsed;

    return j;
}

nlohmann::json toJson(const Resources& res) {
    nlohmann::json j;

    j["gold"] = res.gold;
    j["cube"] = res.cube;
    j["humans"] = res.humans;
    j["winPoints"] = res.winPoints;
    j["gods"] = toJson(res.gods);
    j["books"] = toJson(res.books);

    return j;
}

nlohmann::json toJson(const FederationTile& op) {
    nlohmann::json j;

    j["origin"] = op.origin;
    j["flipped"] = op.flipped;

    return j;
}

nlohmann::json toJson(const BuildingOnMap& op) {
    nlohmann::json j;

    j["type"] = SC(op.type);
    j["owner"] = op.owner;
    j["neutral"] = op.neutral;
    j["hasAnnex"] = op.hasAnnex;
    j["fedIdx"] = op.fedIdx;
    
    return j;
}

nlohmann::json toJson(const Palace& op) {
    nlohmann::json j;

    j["special"] = SC(op.special);
    j["income"] = toJson(op.income);
    j["buttonOrigin"] = op.buttonOrigin;

    return j;
}

nlohmann::json toJson(const BookButton& op) {
    nlohmann::json j;

    j["bookPrice"] = op.bookPrice;
    j["buttonOrigin"] = op.buttonOrigin;
    j["isUsed"] = op.isUsed;
    j["picOrigin"] = op.picOrigin;

    return j;
}

nlohmann::json toJson(const MarketButton& op) {
    nlohmann::json j;

    j["manaPrice"] = op.manaPrice;
    j["buttonOrigin"] = op.buttonOrigin;
    j["isUsed"] = op.isUsed;
    j["picOrigin"] = op.picOrigin;

    return j;
}

int toJson(const TechTile& op) {
    return SC(op);
}

int toJson(const TerrainType& op) {
    return SC(op);
}

int toJson(const Race& op) {
    return SC(op);
}

int toJson(const Innovation& op) {
    return SC(op);
}

int toJson(const Building& op) {
    return SC(op);
}

nlohmann::json toJson(const PlayerState& ps) {
    nlohmann::json j;

    j["resources"] = toJson(ps.resources);
    j["mana"] = ps.mana;
    j["wpPerEvent"] = toJson(ps.wpPerEvent);
    j["techTiles"] = toJson(ps.techTiles);
    j["feds"] = toJson(ps.feds);
    j["buttons"] = toJson(ps.buttons);
    j["innovations"] = toJson(ps.innovations);
    j["buildingsAvailable"] = toJson(ps.buildingsAvailable);
    j["neutralBuildingsAmount"] = toJson(ps.neutralBuildingsAmount);
    j["additionalIncome"] = toJson(ps.additionalIncome);
    j["boosterButton"] = toJson(ps.boosterButton);

    j["boosterButton"] = ps.palaceIdx;
    j["bridgesLeft"] = ps.bridgesLeft;
    j["humansLeft"] = ps.humansLeft;
    j["navLevel"] = ps.navLevel;
    j["tfLevel"] = ps.tfLevel;
    j["annexLeft"] = ps.annexLeft;
    j["currentRoundBoosterOriginIdx"] = ps.currentRoundBoosterOriginIdx;
    j["passed"] = ps.passed;

    return j;
}

nlohmann::json toJson(const Field& f) {
    nlohmann::json j;

    j["type"] = toJson(f.type);
    j["basic_type"] = toJson(StaticData::fieldOrigin().basicType);
    j["bridges"] = toJson(f.bridges);

    std::vector<std::array<int8_t, 2>> bridgesHexes;
    for(const auto& [idx, owner] : enumerate(f.bridges)) {
        if (owner >= 0) {
            bridgesHexes.emplace_back(std::array<int8_t, 2>{StaticData::fieldOrigin().bridgeConnections[idx].first, StaticData::fieldOrigin().bridgeConnections[idx].second});
        }
    }
    j["bridgesHexes"] = toJson(bridgesHexes);
    j["building"] = toJson(f.building);

    return j;
}

nlohmann::json toJson(const StaticGameState& sgs) {
    nlohmann::json j;

    // j["roundBoosters"] = toJson(sgs.roundBoosters);
    j["lastRoundBonus"] = sgs.lastRoundBonus;
    j["bonusByRound"] = toJson(sgs.bonusByRound);
    j["playerRaces"] = toJson(sgs.playerRaces);
    j["playerColors"] = toJson(sgs.playerColors);
    j["techTiles"] = toJson(sgs.techTiles);
    j["bookAndGodPerTech"] = toJson(sgs.bookAndGodPerTech);
    j["palaces"] = toJson(sgs.palaces);
    j["neutralGods"] = toJson(sgs.neutralGods);

    return j;
}

nlohmann::json toJson(const GameState& gs) {
    nlohmann::json j;

    j["activePlayer"] = gs.activePlayer;
    j["round"] = gs.round;
    j["boosters"] = toJson(gs.boosters);
    j["playersOrder"] = gs.playersOrder;

    j["players"] = nlohmann::json::array();
    j["players"].push_back(toJson(gs.players[0]));
    j["players"].push_back(toJson(gs.players[1]));

    j["fedTilesAvailable"] = toJson(gs.fedTilesAvailable);
    j["innovations"] = toJson(gs.innovations);
    j["palacesAvailable"] = toJson(gs.palacesAvailable);
    j["humansOnGods"] = toJson(gs.humansOnGods);
    j["bookActions"] = toJson(gs.bookActions);
    j["marketActions"] = toJson(gs.marketActions);
    j["phase"] = SC(gs.phase);

    j["field"] = toJson(*gs.field);
    j["staticGs"] = toJson(gs.staticGs);

    return j;
}
