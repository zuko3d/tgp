#pragma once

#include "Resources.h"

#include <array>

enum class HexType : uint8_t {
    Desert = 0,
    Plains = 1,
    Swamp = 2,
    Lake = 3,
    Forest = 4,
    Mountain = 5,
    Wasteland = 6,
    River = 7,
    None = 8,
};

using TerrainType = HexType;

enum class LandTypeBonusSpecial {
    Nav,
    TerraformCheap,
    IncomeBetter,
    NoBook2ndInvention,
    None,
};

struct LandTypeBonus {
    IncomableResources resources;
    LandTypeBonusSpecial special = LandTypeBonusSpecial::None;
};

enum Race : uint8_t {
    Blessed = 0,
    Felines = 1,
    Goblins = 2,
    Illusionists = 3,
    Inventors = 4,
    Lizards = 5,
    Moles = 6,
    Monks = 7,
    Navigators = 8,
    Omar = 9,
    Philosophers = 10,
    Psychics = 11,
    None = 12,
};

struct RaceStartBonus {
    IncomableResources resources;
    std::array<int8_t, 4> gods; 
};

enum class BookColor : uint8_t {
    Yellow = 0,
    Blue = 1,
    Brown = 2,
    White = 3,
};

using GodColor = BookColor;

enum class EventType : uint8_t {
    None,
    BuildOnEdge,
    BuildNearRiver,
    BuildExactType,
    BuildHuge,
    PutManToGod,
    MoveGod,
    GetInvention,
    FormFederation,
    Terraform,
    UpgradeNavOrTerra,
};

struct RoundScoreBonus {
    EventType event;
    int eventParams;
    int bonusWp;

    BookColor god;
    int godAmount;
    IncomableResources resourceBonus;
    bool noRound56 = false;
};

enum class Building : uint8_t {
    Mine,
    Guild,
    Palace,
    Laboratory,
    Academy,
    Tower,
    Monument,
    None
};

struct RoundBoosterOrigin {
    IncomableResources resources;
    
    EventType trigger = EventType::None;
    int8_t wpPerTrigger = 0;

    ActionType action = ActionType::Pass;

    bool navBooster = false;
    bool scoreHuge = false;
    bool scoreGuilds = false;
    bool godsForLabs = false;
};

struct RoundBoosterOnBoard {
    const RoundBoosterOrigin& origin;
    uint8_t gold;
};

enum class TechTile : uint8_t {
    BookCharge,
    p2g3,
    cubeGod,
    charge4,
    spades2,
    cube5p2g,
    putGod2p,
    scoreFeds,
    scoreMinGod,
    annex,
    scoreEdge,
    tower,
};

struct FederationTile
{
    uint8_t origin;
    uint8_t flipped = 0;
};

enum class Innovation : uint8_t {
    SpadeBookGods,
    Guild2Wp,
    Human3wpButton,
    GodsAnd10wp,
    Nbuildings,
    Labs5wp,
    Feds5wp,
    Gods2xwp,
    GroupsWp,
    MinesWp,
    HumanNavTerraform,
    Bridges,
    MineAnd3cubes,
    GuildAnd5gold,
    LabAndTech,
    AcademyAnd2wp,
    PalaceAndMana,
    MonumentAnd7wp,
    None,
};

enum class PalaceSpecial : uint8_t {
    DowngradeLabCube3wp,
    UpgradeMine,
    GetTech,
    Lab3wp,
    Fed6nrg,
    Charge12book2,
    Fed,
    Mine2wp,
    Guild3wp,
    Nav2,
    Spades2Books2Bridges2,
    FreeGuild,
    Wp10,
    None,
};

struct Palace {
    IncomableResources income = IncomableResources{};
    IncomableResources button = IncomableResources{};
    PalaceSpecial special = PalaceSpecial::None;
};

enum class BookActionSpecial : uint8_t {
    Guild2wp,
    UpgradeMine,
    None
};

struct BookAction {
    int8_t price;
    IncomableResources resources;
    BookActionSpecial special;
};

struct MarketAction {
    int8_t price;
    IncomableResources resources;
    bool bridge = false;
};

enum class GamePhase {
    Preparation,
    Upkeep,
    Actions,
    EndOfTurn,
};
