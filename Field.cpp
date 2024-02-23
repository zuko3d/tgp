#include "Field.h"

#include "StaticData.h"
#include "Utils.h"
#include <queue>

std::vector<int8_t> Field::buildableBridges(int owner) const {
    std::vector<int8_t> ret;
    std::array<bool, FieldOrigin::TOTAL_BRIDGES> r;
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
    std::array<int8_t, FieldOrigin::FIELD_SIZE> ret = {-1};
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
                        if (building[pos].owner == owner && ret[r] == -1) {
                            q.push(r);
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

    std::array<int, 20> countByComponent = {0};
    for (const auto& c: components) {
        countByComponent[c]++;
    }

    return maximum(countByComponent);
}

ResizableArray<int8_t, 8> Field::adjacent(int pos) const {
    ResizableArray<int8_t, 8> ret;
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
                r[StaticData::fieldOrigin().bridgeConnections[br].first] = true;
                r[StaticData::fieldOrigin().bridgeConnections[br].second] = true;
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
