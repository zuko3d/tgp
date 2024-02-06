#include "StaticData.h"

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
        IncomableResources{.anyGod = 4, .winPoints = 7},
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
        Palace { .income = IncomableResources{ .manaCharge = 5, }, .buttonOrigin = 9 },
        Palace { .income = IncomableResources{}, .buttonOrigin = 8 },
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .buttonOrigin = 7 },
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .buttonOrigin = 6 },
        Palace { .income = IncomableResources{ .manaCharge = 4 }, .special = PalaceSpecial::GetTech },
        Palace { .income = IncomableResources{ .anyBook = 1, .manaCharge = 2 }, .buttonOrigin = 4 },
        Palace { .income = IncomableResources{ .manaCharge = 4 }, .special = PalaceSpecial::Lab3wp },
        Palace { .income = IncomableResources{ .gold = 2, .cube = 1, .manaCharge = 2 }, .special = PalaceSpecial::Fed6nrg },
        Palace { .income = IncomableResources{ .gold = 6 }, .special = PalaceSpecial::Charge12book2 },
        Palace { .income = IncomableResources{ .cube = 1 }, .special = PalaceSpecial::Fed },
        Palace { .income = IncomableResources{ .manaCharge = 8 }, .special = PalaceSpecial::Mine2wp },
        Palace { .income = IncomableResources{ }, .buttonOrigin = 5, .special = PalaceSpecial::Guild3wp },
        Palace { .income = IncomableResources{ .manaCharge = 6 }, .special = PalaceSpecial::Nav2 },
        Palace { .income = IncomableResources{ .manaCharge = 6 }, .special = PalaceSpecial::Spades2Books2Bridges2 },
        Palace { .income = IncomableResources{ .anyBook = 1, .manaCharge = 2 }, .special = PalaceSpecial::FreeGuild },
        Palace { .income = IncomableResources{ .manaCharge = 2 }, .special = PalaceSpecial::Wp10 },
    };
}

std::array<BookButton, 6> StaticData::generateBookActions() {
    return std::array<BookButton, 6> {
        BookButton { .bookPrice = 1, .buttonOrigin = 13 },
        BookButton { .bookPrice = 1, .buttonOrigin = 4 },
        BookButton { .bookPrice = 2, .buttonOrigin = 14 },
        BookButton { .bookPrice = 2, .buttonOrigin = 6 },
        BookButton { .bookPrice = 2, .buttonOrigin = 15 },
        BookButton { .bookPrice = 3, .buttonOrigin = 16 },
    };
};

std::array<MarketButton, 6> StaticData::generateMarketActions() {
    return std::array<MarketButton, 6> {
        MarketButton { .manaPrice = 3, .buttonOrigin = 0 },
        MarketButton { .manaPrice = 3, .buttonOrigin = 11 },
        MarketButton { .manaPrice = 4, .buttonOrigin = 12 },
        MarketButton { .manaPrice = 4, .buttonOrigin = 9 },
        MarketButton { .manaPrice = 4, .buttonOrigin = 1 },
        MarketButton { .manaPrice = 6, .buttonOrigin = 8 },
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
    return {};
}
