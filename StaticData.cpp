#include "StaticData.h"

const std::array<IncomableResources, 7>& StaticData::fedTiles() {
    static const auto ret = generateFedTiles();
    return ret;
}

const std::array<RoundBoosterOrigin, 10>& StaticData::roundBoosters() {
    static const auto ret = generateRoundBoosters();
    return ret;
}

static const std::array<ButtonOrigin, 7>& StaticData::buttonOrigins() {
    static const auto ret = generateButtonOrigins();
    return ret;
}

static const FieldOrigin& StaticData::fieldOrigin() {
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

std::array<InnoPrice, 6> StaticData::generateInnoPrices() {
    return std::array<InnoPrice, 6>{
        InnoPrice{ FlatMap<BookColor, int8_t, 4>{ 2, 2, 0, 0}, .anyBooks = 1 },
        InnoPrice{ FlatMap<BookColor, int8_t, 4>{ 0, 0, 2, 2}, .anyBooks = 1 },
        InnoPrice{ FlatMap<BookColor, int8_t, 4>{ 2, 0, 0, 0}, .anyBooks = 3 },
        InnoPrice{ FlatMap<BookColor, int8_t, 4>{ 0, 2, 0, 0}, .anyBooks = 3 },
        InnoPrice{ FlatMap<BookColor, int8_t, 4>{ 0, 0, 2, 0}, .anyBooks = 3 },
        InnoPrice{ FlatMap<BookColor, int8_t, 4>{ 0, 0, 0, 2}, .anyBooks = 3 },
    };
}

static const std::array<ButtonOrigin, 11>& StaticData::generateButtonOrigins() {
    return std::array<ButtonOrigin, 11> {
        ButtonOrigin {.resources = IncomableResources{}, .special = ButtonActionSpecial::BuildBridge}, // 0
        ButtonOrigin {.resources = IncomableResources{ .spades = 1 }, .special = ButtonActionSpecial::None}, // 1
        ButtonOrigin {.resources = IncomableResources{ .anyGod = 1 }, .special = ButtonActionSpecial::None}, // 2
        ButtonOrigin {.resources = IncomableResources{ .manaCharge = 4 }, .special = ButtonActionSpecial::None}, // 3
        ButtonOrigin {.resources = IncomableResources{ .anyGod = 2 }, .special = ButtonActionSpecial::None}, // 4
        ButtonOrigin {.resources = IncomableResources{ .gold = 3, .anyBook = 1 }, .special = ButtonActionSpecial::None}, // 5
        ButtonOrigin {.resources = IncomableResources{}, .special = ButtonActionSpecial::UpgradeMine}, // 6
        ButtonOrigin {.resources = IncomableResources{}, .special = ButtonActionSpecial::FiraksButton}, // 7
        ButtonOrigin {.resources = IncomableResources{ .spades = 2 }, .special = ButtonActionSpecial::None}, // 8
        ButtonOrigin {.resources = IncomableResources{ .cube = 2 }, .special = ButtonActionSpecial::None}, // 9
        ButtonOrigin {.resources = IncomableResources{ .humans = 1, winPoints = 3 }, .special = ButtonActionSpecial::None}, // 10
    };
}

static FlatMap<Building, BuildingOrigin, 7> StaticData::generateBuildingOrigins() {
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
        IncomableResources{.anyGod = 4, .winPoints = 7},
        IncomableResources{.spades = 2, .winPoints = 5},
        IncomableResources{.manaCharge = 8, .winPoints = 8},
        IncomableResources{.cube = 3, .winPoints = 4},
        IncomableResources{.humans = 1, .winPoints = 8},
        IncomableResources{.gold = 6, .winPoints = 6},
    };
}

const FlatMap<Building, EventType, 8> StaticData::eventPerBuilding() {
    static const auto ret = generateEventPerBuilding();
    return ret;
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
        RoundScoreBonus{.event = EventType::UpgradeNavOrTerra, .bonusWp = 3,
            .god = BookColor::Brown, .godAmount = 3, .resourceBonus = IncomableResources{ .humans = 1 }},
        RoundScoreBonus{.event = EventType::FormFederation, .bonusWp = 5,
            .god = BookColor::Brown, .godAmount = 4, .resourceBonus = IncomableResources{ .spades = 1 }},
        RoundScoreBonus{.event = EventType::BuildHuge, .bonusWp = 5,
            .god = BookColor::White, .godAmount = 2, .resourceBonus = IncomableResources{ .cube = 1 }},
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Guild), .bonusWp = 3,
            .god = BookColor::White, .godAmount = 4, .resourceBonus = IncomableResources{ .spades = 1 }},
        
        RoundScoreBonus{.event = EventType::BuildMine, .bonusWp = 2},
        RoundScoreBonus{.event = EventType::BuildLab, .bonusWp = 4},
        RoundScoreBonus{.event = EventType::BuildGuild, .bonusWp = 3},
        RoundScoreBonus{.event = EventType::BuildOnEdge, .bonusWp = 3},
    };
}

std::array<RoundBoosterOrigin, 10> StaticData::generateRoundBoosters()
{
    return std::array<RoundBoosterOrigin, 10> {
        RoundBoosterOrigin{.navBooster = true},
        RoundBoosterOrigin{.resources = IncomableResources{.humans = 1}, .trigger = EventType::PutManToGod, .wpPerTrigger = 2},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .buttonOriginIdx = 1},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .buttonOriginIdx = 0},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .scoreHuge = true},
        RoundBoosterOrigin{.resources = IncomableResources{.cube = 2}, .buttonOriginIdx = 2},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 2, .manaCharge = 4}},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 6}},
        RoundBoosterOrigin{.resources = IncomableResources{.manaCharge = 3}, .scoreGuilds = true},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 4}, .godsForLabs = true},
    };
}

// static FieldOrigin StaticData::generateFieldOrigin() {}

std::array<RaceStartBonus, 12> StaticData::generateRaceStartBonus() {
    return std::array<RaceStartBonus, 12> {
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {1, 1, 1, 1} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {1, 0, 0, 1} },
        RaceStartBonus{ .resources = IncomableResources{ .cube = 1 }, .gods = {1, 0, 1, 0} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 0, 0, 2} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 0, 0, 0} },
        RaceStartBonus{ .resources = IncomableResources{ .anyGod = 2 }, .gods = {0, 0, 0, 0} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 0, 3, 0} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 1, 0, 0} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {0, 3, 0, 0} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {1, 1, 0, 0} },
        RaceStartBonus{ .resources = IncomableResources{}, .gods = {2, 0, 0, 0} },
        RaceStartBonus{ .resources = IncomableResources{ .cube = 1 }, .gods = {1, 0, 0, 1} }
    };
}

std::array<Palace, 17> StaticData::generatePalaces() {
    return std::array<Palace, 17> {
        Palace { .income = IncomableResources{ .manaCharge = 5, }, .button = IncomableResources{ .cube = 2}, .special = PalaceSpecial::None },
        Palace { .income = IncomableResources{}, .button = IncomableResources{ .spades = 2 }, .special = PalaceSpecial::None },
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .button = IncomableResources{}, .special = PalaceSpecial::DowngradeLabCube3wp },
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .button = IncomableResources{}, .special = PalaceSpecial::UpgradeMine },
        Palace { .income = IncomableResources{ .manaCharge = 4 }, .button = IncomableResources{}, .special = PalaceSpecial::GetTech },
        Palace { .income = IncomableResources{ .anyBook = 1, .manaCharge = 2 }, .button = IncomableResources{. anyGod = 2}, .special = PalaceSpecial::None },
        Palace { .income = IncomableResources{ .manaCharge = 4 }, .button = IncomableResources{}, .special = PalaceSpecial::Lab3wp },
        Palace { .income = IncomableResources{ .gold = 2, .cube = 1, .manaCharge = 2 }, .button = IncomableResources{}, .special = PalaceSpecial::Fed6nrg },
        Palace { .income = IncomableResources{ .gold = 6 }, .button = IncomableResources{}, .special = PalaceSpecial::Charge12book2 },
        Palace { .income = IncomableResources{ .cube = 1 }, .button = IncomableResources{}, .special = PalaceSpecial::Fed },
        Palace { .income = IncomableResources{ .manaCharge = 8 }, .button = IncomableResources{}, .special = PalaceSpecial::Mine2wp },
        Palace { .income = IncomableResources{ }, .button = IncomableResources{ .gold = 3, .anyBook = 1 }, .special = PalaceSpecial::Guild3wp },
        Palace { .income = IncomableResources{ .manaCharge = 6 }, .button = IncomableResources{}, .special = PalaceSpecial::Nav2 },
        Palace { .income = IncomableResources{ .manaCharge = 6 }, .button = IncomableResources{}, .special = PalaceSpecial::Spades2Books2Bridges2 },
        Palace { .income = IncomableResources{ .anyBook = 1, .manaCharge = 2 }, .button = IncomableResources{}, .special = PalaceSpecial::FreeGuild },
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .button = IncomableResources{}, .special = PalaceSpecial::Wp10 },
    };
}

std::array<BookButton, 6> StaticData::generateBookActions() {
    return std::array<BookButton, 6> {
        BookButton { .price = 1, .resources = IncomableResources { .manaCharge = 5 }, .special = BookActionSpecial::None},
        BookButton { .price = 1, .resources = IncomableResources { .anyGod = 2 }, .special = BookActionSpecial::None},
        BookButton { .price = 2, .resources = IncomableResources { .gold = 6 }, .special = BookActionSpecial::None},
        BookButton { .price = 2, .resources = IncomableResources {}, .special = BookActionSpecial::UpgradeMine},
        BookButton { .price = 2, .resources = IncomableResources {}, .special = BookActionSpecial::Guild2wp},
        BookButton { .price = 3, .resources = IncomableResources { .spades = 3 }, .special = BookActionSpecial::None},
    };
};

std::array<MarketButton, 6> StaticData::generateMarketActions() {
    return std::array<MarketButton, 6> {
        MarketButton { .price = 3, .resources = IncomableResources { }, .bridge = true },
        MarketButton { .price = 3, .resources = IncomableResources { .humans = 1 } },
        MarketButton { .price = 4, .resources = IncomableResources { .cube = 2 } },
        MarketButton { .price = 4, .resources = IncomableResources { .gold = 7 } },
        MarketButton { .price = 4, .resources = IncomableResources { .spades = 1 } },
        MarketButton { .price = 6, .resources = IncomableResources { .spades = 2 } },
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

std::array<FieldHex, 13> StaticData::generateFieldHexes()
{
    return std::array<FieldHex, 13>{
        FieldHex{PlanetType::None, PlanetType::None, PlanetType::None, 
            PlanetType::None, PlanetType::None, PlanetType::Blue, PlanetType::Purple,
            PlanetType::None, PlanetType::Brown, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::Yellow, PlanetType::None, PlanetType::None, PlanetType::Orange,
            PlanetType::None, PlanetType::None, PlanetType::Red},

        FieldHex{PlanetType::Black, PlanetType::None, PlanetType::None, 
            PlanetType::Orange, PlanetType::None, PlanetType::White, PlanetType::Yellow,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::Brown, PlanetType::None, PlanetType::Purple,
            PlanetType::None, PlanetType::Red, PlanetType::None},
        
        FieldHex{PlanetType::Purple, PlanetType::None, PlanetType::None, 
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::Black,
            PlanetType::None, PlanetType::Green, PlanetType::None, PlanetType::White, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::Blue, PlanetType::Yellow},
        
        FieldHex{PlanetType::Black, PlanetType::None, PlanetType::None, 
            PlanetType::None, PlanetType::Red, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::Brown, PlanetType::Blue,
            PlanetType::White, PlanetType::Orange, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None},
        
        FieldHex{PlanetType::White, PlanetType::None, PlanetType::Purple, 
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::Red,
            PlanetType::None, PlanetType::Green, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::Orange, PlanetType::Yellow},
        
        FieldHex{PlanetType::None, PlanetType::Purple, PlanetType::None, 
            PlanetType::None, PlanetType::None, PlanetType::Blue, PlanetType::None,
            PlanetType::None, PlanetType::Brown, PlanetType::None, PlanetType::None, PlanetType::Yellow,
            PlanetType::None, PlanetType::None, PlanetType::Green, PlanetType::Purple,
            PlanetType::None, PlanetType::None, PlanetType::None},

        FieldHex{PlanetType::None, PlanetType::Brown, PlanetType::None, 
            PlanetType::None, PlanetType::Red, PlanetType::None, PlanetType::None,
            PlanetType::Purple, PlanetType::None, PlanetType::None, PlanetType::Green, PlanetType::None,
            PlanetType::None, PlanetType::Green, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::Black},
        
        FieldHex{PlanetType::Blue, PlanetType::None, PlanetType::Purple, 
            PlanetType::None, PlanetType::White, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::Black, PlanetType::None,
            PlanetType::None, PlanetType::Orange, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::Purple, PlanetType::None},
        
        FieldHex{PlanetType::None, PlanetType::Purple, PlanetType::White, 
            PlanetType::Orange, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::Green, PlanetType::None,
            PlanetType::None, PlanetType::Black, PlanetType::None, PlanetType::None,
            PlanetType::Brown, PlanetType::None, PlanetType::None},
        
        FieldHex{PlanetType::None, PlanetType::Purple, PlanetType::Purple, 
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::Yellow, PlanetType::None, PlanetType::Green, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::Blue, PlanetType::Red, PlanetType::None},
        
        FieldHex{PlanetType::White, PlanetType::None, PlanetType::Purple, 
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::Red,
            PlanetType::None, PlanetType::Green, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::Orange, PlanetType::None},
        
        FieldHex{PlanetType::None, PlanetType::Purple, PlanetType::None, 
            PlanetType::None, PlanetType::None, PlanetType::Blue, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::None, PlanetType::Yellow,
            PlanetType::None, PlanetType::None, PlanetType::Green, PlanetType::Purple,
            PlanetType::None, PlanetType::None, PlanetType::None},
        
        FieldHex{PlanetType::None, PlanetType::None, PlanetType::None, 
            PlanetType::None, PlanetType::Green, PlanetType::None, PlanetType::None,
            PlanetType::Purple, PlanetType::None, PlanetType::None, PlanetType::Brown, PlanetType::None,
            PlanetType::None, PlanetType::Green, PlanetType::None, PlanetType::None,
            PlanetType::None, PlanetType::None, PlanetType::Black}
    };
}

std::vector<ResizableArray<uint16_t, 6>> StaticData::generateFieldTopology(int mapSize)
{
    std::array<ResizableArray<uint16_t, 6>, 19> internalTopology;
    internalTopology.at(0).fromArray({1, 3, 4}, 3);
    internalTopology.at(1).fromArray({0, 2, 4, 5}, 4);
    internalTopology.at(2).fromArray({1, 5, 6}, 3);
    internalTopology.at(3).fromArray({0, 4, 7, 8}, 4);
    internalTopology.at(4).fromArray({0, 1, 3, 5, 8, 9}, 6);
    internalTopology.at(5).fromArray({0, 2, 4, 6, 9, 10}, 6);
    internalTopology.at(6).fromArray({2, 5, 10, 11}, 4);
    internalTopology.at(7).fromArray({3, 8, 12}, 3);
    internalTopology.at(8).fromArray({3, 4, 7, 9, 12, 13}, 6);
    internalTopology.at(9).fromArray({4, 5, 8, 10, 13, 14}, 6);
    internalTopology.at(10).fromArray({4, 6, 9, 11, 14, 15}, 6);
    internalTopology.at(11).fromArray({6, 10, 15}, 3);
    internalTopology.at(12).fromArray({7, 8, 13, 16}, 4);
    internalTopology.at(13).fromArray({8, 9, 12, 14, 16, 17}, 6);
    internalTopology.at(14).fromArray({9, 10, 13, 15, 17, 18}, 6);
    internalTopology.at(15).fromArray({10, 11, 14, 18}, 4);
    internalTopology.at(16).fromArray({12, 13, 17}, 3);
    internalTopology.at(17).fromArray({13, 14, 16, 18}, 4);
    internalTopology.at(18).fromArray({14, 15, 17}, 3);

    std::vector<ResizableArray<uint16_t, 6>> ret;
    if (mapSize == 7) {
        ret.resize(19 * 7);

        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 19; j++) {
                ret.at(i * 19 + j) = internalTopology.at(j);
                for (auto& p: ret.at(i * 19 + j)) {
                    p += i * 19;
                }
            }
        }

        auto connect = [&] (uint16_t h1, uint16_t c1, uint16_t h2, uint16_t c2) {
            ret.at(h1 * 19 + c1).push_back(h2 * 19 + c2);
            ret.at(h2 * 19 + c2).push_back(h1 * 19 + c1);
        };
        for (const auto [p1, p2]: {{0, 1}, {2, 3}, {3, 4}, {5, 6}}) {
            connect(p1, 2, p2, 7);
            connect(p1, 6, p2, 7);
            connect(p1, 6, p2, 12);
            connect(p1, 11, p2, 12);
            connect(p1, 11, p2, 16);
        }

        for (const auto [p1, p2]: {{0, 3}, {1, 4}, {2, 5}, {3, 6}}) {
            connect(p1, 11, p2, 0);
            connect(p1, 15, p2, 0);
            connect(p1, 15, p2, 3);
            connect(p1, 18, p2, 3);
            connect(p1, 18, p2, 7);
        }

        for (const auto [p1, p2]: {{0, 2}, {1, 3}, {3, 5}, {4, 6}}) {
            connect(p1, 18, p2, 2);
            connect(p1, 17, p2, 1);
            connect(p1, 17, p2, 2);
            connect(p1, 16, p2, 0);
            connect(p1, 16, p2, 1);
        }
    }

    return ret;
}
