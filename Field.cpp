#include "Field.h"

#include "GameState.h"
#include "StaticData.h"
#include "Utils.h"
#include <queue>

std::vector<int8_t> Field::buildableBridges(int owner) const {
    std::vector<int8_t> ret;
    std::array<bool, FieldOrigin::TOTAL_BRIDGES> r {{false}};
    ret.reserve(40);
    for (const auto& pos: ownedByPlayer[owner]) {
        for (const auto& br: StaticData::fieldOrigin().bridgeIds[pos]) {
            if (bridges[br] == -1) r[br] = true;
        }
    }

    for (const auto& [idx, val]: enumerate(r)) {
        if (val) ret.emplace_back(idx);
    }

    return ret;
}

int Field::adjacentEnemiesPower(int8_t pos, int owner) const {
    int opp = 1 - owner;
    int ret = 0;
    for (const auto& neib: adjacent(pos)) {
        if (building.at(neib).owner == opp) {
            ret += StaticData::buildingOrigins()[building.at(neib).type].power;
            if (building.at(neib).hasAnnex) ret++;
        }
    }

    return ret;
}

bool Field::hasAdjacentEnemies(int8_t pos, int owner) const {
    int opp = 1 - owner;

    for (const auto& neib: adjacent(pos)) {
        if (building.at(neib).owner == opp) {
            return true;
        }
    }

    return false;
}

std::vector<int8_t> Field::buildingByPlayer(Building b, int p, bool withNeutrals) const {
    std::vector<int8_t> ret;
    ret.reserve(ownedByPlayer[p].size());
    for (const auto& pos: ownedByPlayer[p]) {
        if (building.at(pos).type == b && (withNeutrals || !building.at(pos).neutral)) {
            ret.emplace_back(pos);
        }
    }

    return ret;
}

std::array<int8_t, FieldOrigin::FIELD_SIZE> Field::bfs(int owner, int reach) const {
    std::array<int8_t, FieldOrigin::FIELD_SIZE> ret;
    std::fill_n(ret.begin(), FieldOrigin::FIELD_SIZE, -1);
    int curComponent = 0;

    for (const auto& start: ownedByPlayer[owner]) {
        if (ret[start] == -1) {
            curComponent++;
            std::queue<int8_t> q;
            q.push(start);

            while (!q.empty()) {
                const auto pos = q.front();
                q.pop();
                if (ret[pos] == -1) {
                    ret[pos] = curComponent;
                    for (const auto& r: StaticData::fieldOrigin().reachable[reach].at(pos)) {
                        if (building[r].owner == owner && ret[r] == -1) {
                            q.push(r);
                        }
                    }
                    for (const auto& br: StaticData::fieldOrigin().bridgeIds[pos]) {
                        if (bridges[br] != -1) {
                            auto r = StaticData::fieldOrigin().bridgeConnections[br].first;
                            if (building[r].owner == owner && ret[r] == -1) {
                                q.push(r);
                            }
                            r = StaticData::fieldOrigin().bridgeConnections[br].second;
                            if (building[r].owner == owner && ret[r] == -1) {
                                q.push(r);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

int Field::countReachableBuildings(int owner, int reach) const {
    const auto components = bfs(owner, reach);

    std::array<int, 20> countByComponent = {{0}};
    for (const auto& c: components) {
        if (c >= 0) countByComponent[c]++;
    }
    assert(sum(countByComponent) == ownedByPlayer[owner].size());

    return maximum(countByComponent);
}

ResizableArray<int8_t, 10> Field::adjacent(int pos) const {
    ResizableArray<int8_t, 10> ret;
    for (const auto& p: StaticData::fieldOrigin().neibs[pos]) {
        ret.push_back(p);
    }

    for (const auto& br: StaticData::fieldOrigin().bridgeIds[pos]) {
        if (bridges[br] != -1) {
            if (pos != StaticData::fieldOrigin().bridgeConnections[br].first) {
                ret.push_back(StaticData::fieldOrigin().bridgeConnections[br].first);
            } else {
                ret.push_back(StaticData::fieldOrigin().bridgeConnections[br].second);
            }
        }
    }

    return ret;
}

std::vector<int8_t> Field::reachable(int owner, int range, TerrainType color) const {
    std::array<bool, FieldOrigin::FIELD_SIZE> r{{false}};
    for (const auto& pos: ownedByPlayer[owner]) {
        for (const auto& p: StaticData::fieldOrigin().reachable[range][pos]) {
            if (building.at(p).owner == -1) r[p] = true;
        }
        for (const auto& br: StaticData::fieldOrigin().bridgeIds[pos]) {
            if (bridges[br] != -1) {
                auto p = StaticData::fieldOrigin().bridgeConnections[br].first;
                if (building.at(p).owner == -1) r[p] = true;

                p = StaticData::fieldOrigin().bridgeConnections[br].second;
                if (building.at(p).owner == -1) r[p] = true;
            }
        }
    }

    std::vector<int8_t> ret;
    if (color == TerrainType::None) {
        for (const auto& [idx, val]: enumerate(r)) {
            if (val) ret.emplace_back(idx);
        }
    } else {
        for (const auto& [idx, val]: enumerate(r)) {
            if (val && (type[idx] == color)) ret.emplace_back(idx);
        }
    }
    
    return ret;
}

void Field::populateField(GameState& gs, FieldActionType action, int pos, int param1, int param2) {
    uint64_t actionHash = stateIdx;
    actionHash *= 2;
    actionHash += gs.activePlayer;
    actionHash *= 8;
    actionHash += SC(action);
    actionHash *= 128;
    actionHash += pos;
    actionHash *= 8;
    actionHash += param1;
    actionHash *= 2;
    actionHash += param2;

    
    if (gs.cache->fieldActionsCache_.contains(actionHash)) {
        gs.fieldStateIdx = gs.cache->fieldActionsCache_.at(actionHash);
        return;
    }

    std::lock_guard<std::mutex> lock(gs.cache->populateFieldMutex_);

    gs.cache->fieldActionsCache_.emplace(actionHash, gs.cache->fieldByState_.size());
    gs.cache->fieldByState_.push_back(gs.field());
    
    auto& newField = gs.cache->fieldByState_.back();
    newField.stateIdx = gs.cache->fieldByState_.size() - 1;
    gs.fieldStateIdx = newField.stateIdx;

    switch (action)
    {
    case FieldActionType::BuildNew:
        assert(newField.building[pos].owner == -1);
        assert(pos >= 0);
        newField.type[pos] = gs.staticGs.playerColors[gs.activePlayer];
        newField.building[pos].owner = gs.activePlayer;
        newField.building[pos].type = (Building)param1;
        newField.building[pos].neutral = param2;
        newField.ownedByPlayer[gs.activePlayer].push_back(pos);
        break;

    case FieldActionType::BuildBridge:
        assert(newField.bridges[pos] == -1);
        newField.bridges[pos] = gs.activePlayer;
        break;

    case FieldActionType::AddAnnex:
        assert(newField.building[pos].owner == gs.activePlayer);
        newField.building[pos].hasAnnex = true;
        break;

    case FieldActionType::ChangeBuildingType:
        assert(newField.building[pos].owner == gs.activePlayer);
        newField.building[pos].type = (Building)param1;
        break;

    case FieldActionType::Terraform:
        assert(newField.building[pos].owner == -1);
        newField.type[pos] = (TerrainType)param1;
        break;

    default:
        assert(false);
        break;
    }

    if (SC(action) <= 3) {
        auto& ps = gs.players[gs.activePlayer];

        std::array<bool, FieldOrigin::FIELD_SIZE> visited = { false };
        int inFedIdx = -1;

        std::queue<int8_t> q;
        if (action == FieldActionType::BuildBridge) {
            if (newField.building.at(StaticData::fieldOrigin().bridgeConnections.at(pos).first).fedIdx >= 0) {
                newField.building.at(StaticData::fieldOrigin().bridgeConnections.at(pos).second).fedIdx = newField.building.at(StaticData::fieldOrigin().bridgeConnections.at(pos).first).fedIdx;
            } else if (newField.building.at(StaticData::fieldOrigin().bridgeConnections.at(pos).second).fedIdx >= 0) {
                newField.building.at(StaticData::fieldOrigin().bridgeConnections.at(pos).first).fedIdx = newField.building.at(StaticData::fieldOrigin().bridgeConnections.at(pos).second).fedIdx;
            } else {
                auto p = StaticData::fieldOrigin().bridgeConnections.at(pos).first;
                if (newField.building[p].owner == gs.activePlayer) q.push(p);

                p = StaticData::fieldOrigin().bridgeConnections.at(pos).second;
                if (newField.building[p].owner == gs.activePlayer) q.push(p);
            }
        } else {
            if (newField.building.at(pos).fedIdx < 0) {
                q.push(pos);
            }
        }

        while (!q.empty()) {
            const auto pos = q.front();
            q.pop();

            if (visited[pos]) {
                continue;
            }

            assert(newField.building[pos].owner == gs.activePlayer);

            if (newField.building.at(pos).fedIdx >= 0) {
                inFedIdx = newField.building.at(pos).fedIdx;
            }

            visited[pos] = true;

            for (const auto& neib : newField.adjacent(pos)) {
                if (newField.building[neib].owner == gs.activePlayer && !visited[neib]) {
                    q.push(neib);
                }
            }
        }

        if (inFedIdx == -1) {
            int totalPower = 0;
            for (const auto& [idx, v] : enumerate(visited)) {
                if (v) {
                    totalPower += StaticData::buildingOrigins()[newField.building.at(idx).type].power;
                    totalPower += newField.building.at(idx).hasAnnex ? 1 : 0;
                }
            }
            if (totalPower >= 7 || (totalPower >= 6 && (ps.palaceIdx >= 0 && StaticData::palaces()[ps.palaceIdx].special == PalaceSpecial::Fed6nrg))) {
                newField.fedsCount[gs.activePlayer]++;
                inFedIdx = newField.fedsCount[gs.activePlayer];
                for (const auto& [idx, v] : enumerate(visited)) {
                    if (v) {
                        newField.building.at(idx).fedIdx = inFedIdx;
                    }
                }

                // log("Federation: " + std::to_string(inFedIdx) + " of power " + std::to_string(totalPower));
            }
        }
        else {
            newField.building.at(pos).fedIdx = inFedIdx;
        }
    }
}

Field& Field::newField(PrecalcCache& cache)
{
    std::lock_guard<std::mutex> lock(cache.populateFieldMutex_);
    cache.fieldByState_.emplace_back();

    auto& field = cache.fieldByState_.back();
    field.stateIdx = cache.fieldByState_.size() - 1;
    field.type = StaticData::fieldOrigin().basicType;
    for (auto& b : field.bridges) b = -1;

    return cache.fieldByState_.back();
}
