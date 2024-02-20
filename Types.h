#pragma once

#include "Resources.h"

#include <array>
#include <stdint.h>
#include <vector>

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

enum class Race : uint8_t {
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
    FlatMap<GodColor, int8_t, 4> gods; 
};

enum class EventType : uint8_t {
    BuildOnEdge,
    BuildNearRiver,
    BuildMine,
    BuildGuild,
    BuildLab,
    BuildHuge,
    PutManToGod,
    MoveGod,
    GetInvention,
    FormFederation,
    Terraform,
    UpgradeNavOrTerra,
    None,
};

struct RoundScoreBonus {
    EventType event = EventType::None;
    // int eventParams;
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

struct BuildingOrigin {
    Building type;
    Resources price = {};
    IncomableResources income = {};
    EventType buildEvent = EventType::None;
    int power = 0;
};

struct BuildingOnMap {
    Building type = Building::None;
    int8_t owner = -1;
    bool neutral = false;
    bool hasAnnex = false;
    int8_t fedIdx = -1;
};

struct RoundBoosterOrigin {
    IncomableResources resources;
    
    EventType trigger = EventType::None;
    int8_t wpPerTrigger = 0;

    int8_t buttonOriginIdx = -1;

    bool navBooster = false;
    bool scoreHuge = false;
    bool scoreGuilds = false;
    bool godsForLabs = false;
};

struct RoundBoosterOnBoard {
    int8_t originIdx;
    int8_t gold;
};

enum class TechTile : uint8_t {
    BookCharge,
    p3g2,
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

using FedTileOrigin = int8_t;

struct FederationTile
{
    FedTileOrigin origin;
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
    FlyingMan,
    None,
};

struct Palace {
    IncomableResources income = IncomableResources{};
    int8_t buttonOrigin = -1;
    PalaceSpecial special = PalaceSpecial::None;
};

enum class ButtonActionSpecial : uint8_t {
    FiraksButton,
    BuildBridge,
    UpgradeMine,
    WpForGuilds2,
    None,
};

struct ButtonOrigin {
    IncomableResources resources = IncomableResources{};
    ButtonActionSpecial special = ButtonActionSpecial::None;
};

struct Button {
    int8_t buttonOrigin = -1;
    int8_t isUsed = false;
};

struct MarketButton {
    int8_t manaPrice = 0;
    int8_t buttonOrigin = -1;
    int8_t isUsed = false;
    int8_t picOrigin = -1;
};

struct BookButton {
    int8_t bookPrice = 0;
    int8_t buttonOrigin = -1;
    int8_t isUsed = false;
    int8_t picOrigin = -1;
};

enum class GamePhase {
    Preparation,
    Upkeep,
    Actions,
    EndOfTurn,
};
