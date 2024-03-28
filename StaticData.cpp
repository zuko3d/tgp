#include "StaticData.h"

#include <queue>

const std::array<IncomableResources, 7>& StaticData::fedTiles() {
    static const auto ret = generateFedTiles();
    return ret;
}

const std::array<RoundBoosterOrigin, 10>& StaticData::roundBoosters() {
    static const auto ret = generateRoundBoosters();
    return ret;
}

const std::vector<ButtonOrigin>& StaticData::buttonOrigins() {
    static const auto ret = generateButtonOrigins();
    return ret;
}

const FieldOrigin& StaticData::fieldOrigin() {
    static const auto ret = generateFieldOrigin();
    return ret;
}

const std::array<Palace, 17>& StaticData::palaces() {
    static const auto ret = generatePalaces();
    return ret;
}

const FlatMap<Building, BuildingOrigin, 7>& StaticData::buildingOrigins() {
    static const auto ret = generateBuildingOrigins();
    return ret;
}

const std::array<InnoPrice, 6>& StaticData::innoPrices() {
    static const auto ret = generateInnoPrices();
    return ret;
}

const std::array<RoundScoreBonus, 16>& StaticData::roundScoreBonuses() {
    static const auto ret = generateRoundScoreBonuses();
    return ret;
}

std::array<InnoPrice, 6> StaticData::generateInnoPrices() {
    return std::array<InnoPrice, 6>{
        InnoPrice{ .books = { 2, 2, 0, 0 }, .anyBooks = 1 },
        InnoPrice{ .books = { 0, 0, 2, 2}, .anyBooks = 1 },
        InnoPrice{ .books = { 2, 0, 0, 0}, .anyBooks = 3 },
        InnoPrice{ .books = { 0, 2, 0, 0}, .anyBooks = 3 },
        InnoPrice{ .books = { 0, 0, 2, 0}, .anyBooks = 3 },
        InnoPrice{ .books = { 0, 0, 0, 2}, .anyBooks = 3 },
    };
}

std::vector<ButtonOrigin> StaticData::generateButtonOrigins() {
    return {
        ButtonOrigin { .resources = IncomableResources{}, .special = ButtonActionSpecial::BuildBridge}, // 0
        ButtonOrigin { .resources = IncomableResources{ .spades = 1 }, .special = ButtonActionSpecial::None}, // 1
        ButtonOrigin { .resources = IncomableResources{ .anyGod = 1 }, .special = ButtonActionSpecial::None}, // 2
        ButtonOrigin { .resources = IncomableResources{ .manaCharge = 4 }, .special = ButtonActionSpecial::None}, // 3
        ButtonOrigin { .resources = IncomableResources{ .anyGod = 2 }, .special = ButtonActionSpecial::None}, // 4
        ButtonOrigin { .resources = IncomableResources{ .gold = 3, .anyBook = 1 }, .special = ButtonActionSpecial::None}, // 5
        ButtonOrigin { .resources = IncomableResources{}, .special = ButtonActionSpecial::UpgradeMine}, // 6
        ButtonOrigin { .resources = IncomableResources{}, .special = ButtonActionSpecial::FiraksButton}, // 7
        ButtonOrigin { .resources = IncomableResources{ .spades = 2 }, .special = ButtonActionSpecial::None}, // 8
        ButtonOrigin { .resources = IncomableResources{ .cube = 2 }, .special = ButtonActionSpecial::None}, // 9
        ButtonOrigin { .resources = IncomableResources{ .humans = 1, .winPoints = 3 }, .special = ButtonActionSpecial::None}, // 10
        ButtonOrigin { .resources = IncomableResources{ .humans = 1 }, .special = ButtonActionSpecial::None}, // 11
        ButtonOrigin { .resources = IncomableResources{ .gold = 7 }, .special = ButtonActionSpecial::None}, // 12
        ButtonOrigin { .resources = IncomableResources{ .manaCharge = 5 }, .special = ButtonActionSpecial::None}, // 13
        ButtonOrigin { .resources = IncomableResources{ .gold = 6 }, .special = ButtonActionSpecial::None}, // 14
        ButtonOrigin { .resources = IncomableResources{}, .special = ButtonActionSpecial::WpForGuilds2}, // 15
        ButtonOrigin { .resources = IncomableResources{ .spades = 3 }, .special = ButtonActionSpecial::None}, // 16
        ButtonOrigin { .resources = IncomableResources{ .anyBook = 1 }, .special = ButtonActionSpecial::None}, // 17
    };
}

FlatMap<Building, BuildingOrigin, 7> StaticData::generateBuildingOrigins() {
    FlatMap<Building, BuildingOrigin, 7> ret;
    ret[Building::Mine] = BuildingOrigin{ .type = Building::Mine, .price = Resources{ .gold = 2, .cube = 1 }, .income = IncomableResources { .cube = 1 }, .buildEvent = EventType::BuildMine, .power = 1 };
    ret[Building::Guild] = BuildingOrigin{ .type = Building::Guild, .price = Resources{ .gold = 3, .cube = 2 }, .income = IncomableResources { .gold = 2, .manaCharge = 1 }, .buildEvent = EventType::BuildGuild, .power = 2 };
    ret[Building::Palace] = BuildingOrigin{ .type = Building::Palace, .price = Resources{ .gold = 6, .cube = 4 }, .buildEvent = EventType::BuildHuge, .power = 3 };
    ret[Building::Laboratory] = BuildingOrigin{ .type = Building::Laboratory, .price = Resources{ .gold = 5, .cube = 3 }, .income = IncomableResources { .humans = 1 }, .buildEvent = EventType::BuildLab , .power = 2};
    ret[Building::Academy] = BuildingOrigin{ .type = Building::Academy, .price = Resources{ .gold = 8, .cube = 5 }, .income = IncomableResources { .humans = 1 }, .buildEvent = EventType::BuildHuge, .power = 3 };
    ret[Building::Tower] = BuildingOrigin{ .type = Building::Tower, .income = IncomableResources { .gold = 2, .manaCharge = 2 }, .power = 2 };
    ret[Building::Monument] = BuildingOrigin{ .type = Building::Monument, .power = 4 };

    return ret;
}

std::array<IncomableResources, 7> StaticData::generateFedTiles()
{
    return std::array<IncomableResources, 7> {
        IncomableResources{.anyBook = 2, .winPoints = 5},
        IncomableResources{.anyGod = -20, .winPoints = 7},
        IncomableResources{.spades = 2, .winPoints = 5},
        IncomableResources{.manaCharge = 8, .winPoints = 8},
        IncomableResources{.cube = 3, .winPoints = 4},
        IncomableResources{.humans = 1, .winPoints = 8},
        IncomableResources{.gold = 6, .winPoints = 6},
    };
}

std::array<RoundScoreBonus, 16> StaticData::generateRoundScoreBonuses()
{
    return std::array<RoundScoreBonus, 16> {
        RoundScoreBonus{.event = EventType::BuildLab, .bonusWp = 4,
            .god = BookColor::Yellow, .godAmount = 1, .resourceBonus = IncomableResources{ .gold = 1 }},
        RoundScoreBonus{.event = EventType::BuildHuge, .bonusWp = 5,
            .god = BookColor::Yellow, .godAmount = 2, .resourceBonus = IncomableResources{ .cube = 1 }},
        RoundScoreBonus{.event = EventType::BuildMine, .bonusWp = 2,
            .god = BookColor::Yellow, .godAmount = 3, .resourceBonus = IncomableResources{ .manaCharge = 4 }},
        RoundScoreBonus{.event = EventType::GetInvention, .bonusWp = 5,
            .god = BookColor::Blue, .godAmount = 2, .resourceBonus = IncomableResources{ .manaCharge = 3 }},
        RoundScoreBonus{.event = EventType::BuildMine, .bonusWp = 2,
            .god = BookColor::Blue, .godAmount = 3, .resourceBonus = IncomableResources{ .humans = 1 }},
        RoundScoreBonus{.event = EventType::MoveGod, .bonusWp = 1,
            .god = BookColor::White, .godAmount = 3, .resourceBonus = IncomableResources{ .anyBook = 1 }},
        RoundScoreBonus{.event = EventType::BuildGuild, .bonusWp = 3,
            .god = BookColor::Blue, .godAmount = 3, .resourceBonus = IncomableResources{ .anyBook = 1 }},
        RoundScoreBonus{.event = EventType::Terraform, .bonusWp = 2,
            .god = BookColor::Brown, .godAmount = 1, .resourceBonus = IncomableResources{ .gold = 1 }},
        RoundScoreBonus{.event = EventType::UpgradeNavOrTerra, .bonusWp = 3,
            .god = BookColor::Brown, .godAmount = 3, .resourceBonus = IncomableResources{ .humans = 1 }},
        RoundScoreBonus{.event = EventType::FormFederation, .bonusWp = 5,
            .god = BookColor::Brown, .godAmount = 4, .resourceBonus = IncomableResources{ .spades = 1 }},
        RoundScoreBonus{.event = EventType::BuildHuge, .bonusWp = 5,
            .god = BookColor::White, .godAmount = 2, .resourceBonus = IncomableResources{ .cube = 1 }},
        RoundScoreBonus{.event = EventType::BuildGuild, .bonusWp = 3,
            .god = BookColor::White, .godAmount = 4, .resourceBonus = IncomableResources{ .spades = 1 }},
        
        RoundScoreBonus{.event = EventType::BuildMine, .bonusWp = 2},
        RoundScoreBonus{.event = EventType::BuildLab, .bonusWp = 4},
        RoundScoreBonus{.event = EventType::BuildOnEdge, .bonusWp = 3},
        RoundScoreBonus{.event = EventType::BuildGuild, .bonusWp = 3},
    };
}

std::array<RoundBoosterOrigin, 10> StaticData::generateRoundBoosters()
{
    return std::array<RoundBoosterOrigin, 10> {
        RoundBoosterOrigin{.trigger = EventType::BuildNearRiver, .wpPerTrigger = 2, .navBooster = true },
        RoundBoosterOrigin{.resources = IncomableResources{.humans = 1}, .trigger = EventType::PutManToGod, .wpPerTrigger = 2},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .buttonOriginIdx = 1},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .buttonOriginIdx = 0},
        RoundBoosterOrigin{.resources = IncomableResources{.cube = 1}, .scoreHuge = true },
        RoundBoosterOrigin{.resources = IncomableResources{.cube = 2}, .buttonOriginIdx = 2},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 2, .manaCharge = 4}},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 6}},
        RoundBoosterOrigin{.resources = IncomableResources{.manaCharge = 3}, .trigger = EventType::BuildGuild, .wpPerTrigger = 3 },
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 4}, .godsForLabs = true},
    };
}

// static FieldOrigin StaticData::generateFieldOrigin() {}

std::array<RaceStartBonus, 12> StaticData::generateRaceStartBonus() {
    return std::array<RaceStartBonus, 12> {
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {1, 1, 1, 1} }, // Blessed
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {1, 0, 0, 1} }, // Felines
        RaceStartBonus{ .resources = IncomableResources{ .cube = 1 }, .gods = {1, 0, 1, 0} }, // Goblins
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 0, 0, 2} }, // Illusionists
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 0, 0, 0} }, // Inventors
        RaceStartBonus{ .resources = IncomableResources{ .anyGod = -2 }, .gods = {0, 0, 0, 0} }, // Lizards
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 0, 2, 0} }, // Moles
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 1, 0, 0} }, // Monks
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 3, 0, 0} }, // Navigators
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {1, 0, 1, 0} }, // Omar
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {2, 0, 0, 0} }, // Philosophers
        RaceStartBonus{ .resources = IncomableResources{ .cube = 1 }, .gods = {1, 0, 0, 1} } // Psychics
    };
}

std::array<Palace, 17> StaticData::generatePalaces() {
    return std::array<Palace, 17> {
        Palace { .income = IncomableResources{ .manaCharge = 5, }, .buttonOrigin = 9 }, // 0
        Palace { .income = IncomableResources{}, .buttonOrigin = 8 }, // 1
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .buttonOrigin = 7 }, // 2
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .buttonOrigin = 6 }, // 3
        Palace { .income = IncomableResources{ .manaCharge = 4 }, .special = PalaceSpecial::GetTech }, // 4
        Palace { .income = IncomableResources{ .anyBook = 1, .manaCharge = 2 }, .buttonOrigin = 4 }, // 5
        Palace { .income = IncomableResources{ .manaCharge = 4 }, .special = PalaceSpecial::Lab3wp }, // 6
        Palace { .income = IncomableResources{ .gold = 2, .cube = 1, .manaCharge = 2 }, .special = PalaceSpecial::Fed6nrg }, // 7
        Palace { .income = IncomableResources{ .humans = 1 }, .special = PalaceSpecial::FlyingMan }, // 8
        Palace { .income = IncomableResources{ .gold = 6 }, .special = PalaceSpecial::Charge12book2 }, // 9
        Palace { .income = IncomableResources{ .cube = 1 }, .special = PalaceSpecial::Fed }, // 10
        Palace { .income = IncomableResources{ .manaCharge = 8 }, .special = PalaceSpecial::Mine2wp }, // 11
        Palace { .income = IncomableResources{ }, .buttonOrigin = 5, .special = PalaceSpecial::Guild3wp }, // 12
        Palace { .income = IncomableResources{ .manaCharge = 6 }, .special = PalaceSpecial::Nav2 }, // 13
        Palace { .income = IncomableResources{ .manaCharge = 6 }, .special = PalaceSpecial::Spades2Books2Bridges2 }, // 14
        Palace { .income = IncomableResources{ .anyBook = 1, .manaCharge = 2 }, .special = PalaceSpecial::FreeGuild }, // 15
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .special = PalaceSpecial::Wp10 }, // 16
    };
}

std::array<BookButton, 6> StaticData::generateBookActions() {
    return std::array<BookButton, 6> {
        BookButton { .bookPrice = 1, .buttonOrigin = 13, .picOrigin = 0 },
        BookButton { .bookPrice = 1, .buttonOrigin = 4, .picOrigin = 1 },
        BookButton { .bookPrice = 2, .buttonOrigin = 15, .picOrigin = 2 },
        BookButton { .bookPrice = 3, .buttonOrigin = 6, .picOrigin = 3 },
        BookButton { .bookPrice = 2, .buttonOrigin = 14, .picOrigin = 4 },
        BookButton { .bookPrice = 3, .buttonOrigin = 16, .picOrigin = 5 },
    };
};

std::array<MarketButton, 6> StaticData::generateMarketActions() {
    return std::array<MarketButton, 6> {
        MarketButton { .manaPrice = 3, .buttonOrigin = 0, .picOrigin = 0 },
        MarketButton { .manaPrice = 3, .buttonOrigin = 11, .picOrigin = 1 },
        MarketButton { .manaPrice = 4, .buttonOrigin = 9, .picOrigin = 2 },
        MarketButton { .manaPrice = 4, .buttonOrigin = 12, .picOrigin = 3 },
        MarketButton { .manaPrice = 4, .buttonOrigin = 1, .picOrigin = 4 },
        MarketButton { .manaPrice = 6, .buttonOrigin = 8, .picOrigin = 5 },
    };
}

std::array<LandTypeBonus, 7> StaticData::generateLandTypeBonuses() {
    return std::array<LandTypeBonus, 7> {
        LandTypeBonus {.resources = IncomableResources { .spades = 1 }, .special = LandTypeBonusSpecial::None },
        LandTypeBonus {.resources = IncomableResources {}, .special = LandTypeBonusSpecial::TerraformCheap },
        LandTypeBonus {.resources = IncomableResources { .humans = 1, .manaCharge = 2 }, .special = LandTypeBonusSpecial::None },
        LandTypeBonus {.resources = IncomableResources {}, .special = LandTypeBonusSpecial::Nav },
        LandTypeBonus {.resources = IncomableResources { .anyGod = -20, .manaCharge = 1 }, .special = LandTypeBonusSpecial::None },
        LandTypeBonus {.resources = IncomableResources {}, .special = LandTypeBonusSpecial::IncomeBetter },
        LandTypeBonus {.resources = IncomableResources { .cube = 1, .anyBook = 1 }, .special = LandTypeBonusSpecial::NoBook2ndInvention },
    };
};

FieldOrigin StaticData::generateFieldOrigin() {
    FieldOrigin fld;

    fld.basicType = {
        TerrainType::Forest, TerrainType::Mountain, TerrainType::Desert, TerrainType::Plains, TerrainType::River, TerrainType::Lake, TerrainType::Forest, TerrainType::Mountain, TerrainType::Wasteland, TerrainType::River,
        TerrainType::River, TerrainType::Swamp, TerrainType::Lake, TerrainType::Swamp, TerrainType::Wasteland, TerrainType::River, TerrainType::Swamp, TerrainType::Plains, TerrainType::Lake, TerrainType::River, TerrainType::Forest,
        TerrainType::River, TerrainType::Plains, TerrainType::Mountain, TerrainType::Forest, TerrainType::River, TerrainType::River, TerrainType::Desert, TerrainType::River, TerrainType::River, TerrainType::Desert,
        TerrainType::River, TerrainType::Wasteland, TerrainType::Desert, TerrainType::River, TerrainType::Forest, TerrainType::River, TerrainType::River, TerrainType::Mountain, TerrainType::Plains, TerrainType::Swamp,
        TerrainType::River, TerrainType::River, TerrainType::River, TerrainType::Swamp, TerrainType::Plains, TerrainType::Lake, TerrainType::Desert, TerrainType::Wasteland, TerrainType::Mountain,
        TerrainType::River, TerrainType::Lake, TerrainType::Plains, TerrainType::River, TerrainType::Wasteland, TerrainType::Mountain, TerrainType::River, TerrainType::Forest, TerrainType::Lake, TerrainType::Desert,
        TerrainType::River, TerrainType::Swamp, TerrainType::Mountain, TerrainType::River, TerrainType::River, TerrainType::River, TerrainType::River, TerrainType::River, TerrainType::River, TerrainType::River,
        TerrainType::River, TerrainType::Desert, TerrainType::Forest, TerrainType::Wasteland, TerrainType::Desert, TerrainType::Forest, TerrainType::Lake, TerrainType::Swamp, TerrainType::Forest, TerrainType::River,
        TerrainType::River, TerrainType::Wasteland, TerrainType::Plains, TerrainType::Mountain, TerrainType::Lake, TerrainType::Swamp, TerrainType::Plains, TerrainType::Desert, TerrainType::Wasteland, TerrainType::Mountain, TerrainType::River
    };

    std::vector<int> onEdge = {0, 1, 2, 3, 5, 6, 7, 8, 20, 30, 40, 49, 59, 81, 82, 83, 84, 85, 86, 87, 88, 89};
    for (auto& f: fld.onEdge_) { f = false; }
    for (const auto& pos: onEdge) {
        fld.onEdge_[pos] = true;
    }

    std::vector<int> nearRiver = {0, 3, 5, 8, 11, 14, 16, 18, 20, 22, 24, 27, 30, 32, 33, 35, 38, 39, 44, 45, 46, 47, 51, 52, 54, 55, 57, 58, 59, 61, 62, 71, 73, 74, 75, 76, 77, 78, 81, 89};
    for (auto& f: fld.isNearRiver) { f = false; }
    for (const auto& pos: nearRiver) {
        fld.isNearRiver[pos] = true;
    }

    fld.bridgeConnections[1] = {-1, -1};
    fld.bridgeConnections[1] = {5, 14};
    fld.bridgeConnections[2] = {8, 20};
    fld.bridgeConnections[3] = {18, 30};
    fld.bridgeConnections[4] = {18, 38};
    fld.bridgeConnections[5] = {24, 35};
    fld.bridgeConnections[6] = {27, 35};
    fld.bridgeConnections[7] = {27, 38};
    fld.bridgeConnections[8] = {27, 46};
    fld.bridgeConnections[9] = {33, 44};
    fld.bridgeConnections[10] = {32, 51};
    fld.bridgeConnections[11] = {33, 52};
    fld.bridgeConnections[12] = {44, 52};
    fld.bridgeConnections[13] = {54, 62};
    fld.bridgeConnections[14] = {54, 74};
    fld.bridgeConnections[15] = {55, 75};
    fld.bridgeConnections[16] = {57, 77};
    fld.bridgeConnections[17] = {58, 78};

    for (const auto& [idx, p]: enumerate(fld.bridgeConnections)) {
        if (idx > 0) {
            fld.bridgeIds[p.first].push_back(idx);
            fld.bridgeIds[p.second].push_back(idx);
        }
    }

    std::vector<int> rs = {0, 10, 21, 31, 41, 50, 60, 70, 80};
    std::vector<int> re = {9, 20, 30, 40, 49, 59, 69, 79, 90};

    for (size_t row = 0; row < 9; row++) {
        for (int idx = rs[row]; idx < re[row]; idx++) {
            fld.neibs[idx].push_back(idx + 1);
            fld.neibs[idx + 1].push_back(idx);
        }
    }

    std::vector<int> start = {10, 21, 31, 41, 50, 60, 70, 80};
    std::vector<int> end = {19, 30, 39, 49, 58, 68, 79, 89};
    std::vector<int> shift = {10, 10, 9, 9, 9, 9, 10, 10};

    for (size_t row = 0; row < 8; row++) {
        for (int idx = start[row]; idx <= end[row]; idx++) {
            fld.neibs[idx].push_back(idx - shift[row]);
            fld.neibs[idx - shift[row]].push_back(idx);
        }
    }

    start = {11, 21, 31, 41, 51, 60, 71, 81};
    end = {20, 30, 40, 49, 59, 69, 79, 90};
    shift = {11, 11, 10, 10, 10, 10, 11, 11};

    for (size_t row = 0; row < 8; row++) {
        for (int idx = start[row]; idx <= end[row]; idx++) {
            fld.neibs[idx].push_back(idx - shift[row]);
            fld.neibs[idx - shift[row]].push_back(idx);
        }
    }

    for (int startPos = 0; startPos < FieldOrigin::FIELD_SIZE; startPos++) {
        if (fld.basicType[startPos] == TerrainType::River) continue;

        std::queue<std::pair<int, int>> q;
        q.push({startPos, 0});

        std::vector<int> visited(FieldOrigin::FIELD_SIZE, 20);
        
        for (const auto& neib: fld.neibs[startPos]) {
            if (fld.basicType[neib] != TerrainType::River) {
                visited[neib] = 0;
            }
        }
        
        while (!q.empty()) {
            const auto [cur, curWeight] = q.front();
            q.pop();
            if (visited[cur] <= curWeight) continue;
            visited[cur] = curWeight;
            for (const auto& neib: fld.neibs[cur]) {
                if (fld.basicType[neib] == TerrainType::River) {
                    if (visited[neib] > curWeight + 1) {
                        q.push({neib, curWeight + 1});
                    }
                } else {
                    visited[neib] = std::min(visited[neib], curWeight + 1);
                }
            }
        }

        for (const auto& [idx, range]: enumerate(visited)) {
            for (int i = std::max(0, range - 1); i < 5; i++) {
                if (fld.basicType[idx] != TerrainType::River) {
                    fld.reachable.at(i).at(startPos).push_back(idx);
                }
            }
        }
    }
    
    return fld;
}
