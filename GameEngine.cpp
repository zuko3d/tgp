#include "GameEngine.h"

#include "Utils.h"

int spadesNeeded(TerrainType src, TerrainType dst) {
    int dist = (SC(dst) + 7 - SC(src)) % 7;
    dist = std::min(dist, 7 - dist);
    return dist;
}

int GameEngine::charge(int amount, GameState& gs) {
    auto& ps = gs.players.at(gs.activePlayer);

    int charge01 = std::min(amount, ps.mana[0]);
    ps.mana[0] -= charge01;
    ps.mana[1] += charge01;
    amount -= charge01;

    int charge12 = 0;
    if (amount > 0) {
        charge12 = std::min(amount, ps.mana[1]);
        ps.mana[1] -= charge12;
        ps.mana[2] += charge12;
    }

    return charge01 + charge12;
}

int GameEngine::moveGod(int amount, GodColor godColor, GameState& gs) {
    auto& ps = gs.players.at(gs.activePlayer);
    auto& oppPs = gs.players.at(1 - gs.activePlayer);

    const size_t i = SC(godColor);
    int godCharges = 0;
    if ((ps.gods[i] <= 2) && (ps.gods[i] + amount > 2)) {
        godCharges += 1;
    }
    if ((ps.gods[i] <= 4) && (ps.gods[i] + amount > 4)) {
        godCharges += 2;
    }
    if ((ps.gods[i] <= 6) && (ps.gods[i] + amount > 6)) {
        godCharges += 2;
    }
    if ((ps.gods[i] <= 11) && (ps.gods[i] + amount > 11) && (oppPs.gods[i] != 12)) {
        godCharges += 3;
    }

    ps.resources.gods[i] += amount;
    if (ps.resources.gods[i] > 11) {
        if (oppPs.gods[i] == 12) {
            ps.resources.gods[i] = 11;
        } else {
            ps.resources.gods[i] = 12;
        }        
    }

    charge(godCharges, gs);

    return godCharges;
}

void GameEngine::awardResources(IncomableResources resources, GameState& gs) {
    auto& ps = gs.players.at(gs.activePlayer);
    const auto bot = bots_.at(gs.activePlayer);

    if (resources.anyGod != 0) {
        if (resources.anyGod > 0) {
            const size_t i = SC(bot->chooseGodToMove(gs, resources.anyGod));
            moveGod(resources.anyGod, (GodColor) i, gs);
        }

        if (resources.anyGod > -10 && resources.anyGod < 0) {
            assert(false);
        }

        if (resources.anyGod == -20) {
            for (size_t i = 0; i < 4; i++) {
                moveGod(1, (GodColor) i, gs);
            }
        }
    }

    if (resources.anyBook > 0) {
        const auto color = SC(bot->chooseBookColor(gs, resources.anyBook));
        ps.resources.books[color] += resources.anyBook;
    }

    if (resources.manaCharge > 0) {
        charge(resources.manaCharge, gs);
    }

    ps.resources.cube += resources.cube;
    ps.resources.gold += resources.gold;
    ps.resources.humans += resources.humans;
    ps.resources.winPoints += resources.winPoints;

    if (resources.spades > 0) {
        useSpades(resources.spades, gs);
    }
}

void GameEngine::useSpades(int amount, GameState& gs) {
    auto& ps = gs.players.at(gs.activePlayer);
    const auto pColor = gs.staticGs.playerColors[gs.activePlayer];
    const auto bot = bots_.at(gs.activePlayer);

    int spareSpades = amount;

    const auto pos = bot->choosePlaceToSpade(gs, spareSpades);
    const auto needed = spadesNeeded(gs.field->terrainType(pos), pColor);

    if (spareSpades < needed) {
        terraform(pos, spareSpades, gs);
        if (gs.phase == GamePhase::Actions) {
            const auto bricks = bot->chooseBricks(gs, pos);
            if (bricks > 0) {
                awardResources(Resources{.cube = (int8_t) -bricks}, gs);
                terraform(pos, needed - spareSpades, gs);
            }
        }
        spareSpades = 0;
    }

    while (spareSpades > 0) {
        const auto pos = bot->choosePlaceToSpade(gs, spareSpades);
        const auto needed = spadesNeeded(gs.field->terrainType(pos), pColor);
        const auto used = std::min(spareSpades, needed);
        terraform(pos, used, gs);
        spareSpades -= used;
    }

    if (gs.phase == GamePhase::Actions && (gs.field->terrainType(pos) == pColor) && ps.resources.cube >= 1 && ps.resources.gold >= 2) {
        if (bot->wannaBuildMine(gs, pos)) {
            build(pos, Building::Mine, gs);
        }
    }
}

void GameEngine::awardResources(Resources resources, GameState& gs) {
    auto& ps = gs.players.at(gs.activePlayer);

    for (size_t i = 0; i < 4; i++) {
        if (resources.gods[i] > 0) {
            moveGod(resources.gods[i], (GodColor) i, gs);
        }
    }

    ps.resources += resources;
}

void GameEngine::initializeRandomly(GameState& gs, std::vector<IBot *> bots, std::default_random_engine& g) {
    size_t nPlayers = bots.size();

    for (size_t i = 0; i < nPlayers; ++i) {
        gs.playersOrder.at(i) = i;
    }

    gs.activePlayer = 0;
    gs.round = 0; // pre-game
    
    gs.playerPassed = {false, false};

    const auto fedTiles = generateFedTiles();
    gs.fedTilesAvailable = {3, 3, 3, 3, 3, 3};

    std::vector<int> indices;

    const auto allRoundBonuses = generateRoundScoreBonuses();
    indices.resize(12);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 6; ++i) {
        gs.staticGs.bonusByRound.at(i) = allRoundBonuses.at(indices.at(i));
    }
    int counter = 6;
    while (gs.staticGs.bonusByRound.at(4).event == EventType::Terraform) {
        gs.staticGs.bonusByRound.at(4) = allRoundBonuses.at(indices.at(counter));
        counter++;
    }
    while (gs.staticGs.bonusByRound.at(5).event == EventType::Terraform) {
        gs.staticGs.bonusByRound.at(5) = allRoundBonuses.at(indices.at(counter));
        counter++;
    }

    gs.staticGs.lastRoundBonus = allRoundBonuses.at(12 + g() % 4);

    gs.humansOnGods = {
        std::array<uint8_t, 3>{2, 2, 2},
        std::array<uint8_t, 3>{2, 2, 2},
        std::array<uint8_t, 3>{2, 2, 2},
        std::array<uint8_t, 3>{2, 2, 2}
    };

    const auto bookActions = generateBookActions();
    indices.resize(bookActions.size());
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 3; ++i) {
        gs.staticGs.bookActions.at(i) = bookActions.at(indices.at(i));
    }

    gs.staticGs.marketActions = generateMarketActions();

    indices.resize(12);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 12; ++i) {
        gs.staticGs.techTiles.at(i / 3).at(i % 4) = static_cast<TechTile>(indices.at(i));
    }

    indices.resize(18);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 6; ++i) {
        gs.innovations.at(i) = static_cast<Innovation>(indices.at(i));
    }
    
    const auto palaces = generatePalaces();
    indices.resize(16);
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 3; i++) {
        gs.palacesAvailable.push_back(indices.at(i));
    }
    gs.palacesAvailable.push_back(16);


    const auto allRoundBoosters = generateRoundBooosters();
    indices.resize(allRoundBoosters.size());
    std::iota(indices.begin(), indices.end(), 0);
    rshuffle(indices, g);
    for (int i = 0; i < 5; ++i) {
        gs.staticGs.roundBoosters.at(i) = allRoundBoosters.at(indices.at(i));
    }

    std::vector<Race> races;
    for (int i = 0; i < 12; i++) {
        races.emplace_back((Race) i);
    };
    for (int i = 0; i < 2; i++) {
        const Race race = bots_[i]->chooseRace(gs, races);
        gs.staticGs.playerRaces[i] = race;
        std::remove(races.begin(), races.end(), race);
    }

    std::vector<TerrainType> colors;
    for (int i = 0; i < 7; i++) {
        colors.emplace_back((TerrainType) i);
    };
    for (int i = 0; i < 2; i++) {
        const TerrainType color = bots_[i]->chooseTerrainType(gs, colors);
        gs.staticGs.playerColors[i] = color;
        std::remove(colors.begin(), colors.end(), color);
    }

    const auto raceStartBonuses = generateRaceStartBonus();
    for (int i = 0; i < 2; ++i) {
        gs.activePlayer = i;
        const auto raceIdx = gs.staticGs.playerRaces.at(i);
        awardResources(raceStartBonuses[raceIdx].resources, gs);
        awardResources(Resources{.gods = raceStartBonuses[raceIdx].gods}, gs);
    }

    const auto landTypeBonuses = generateLandTypeBonuses();
    for (int i = 0; i < 2; ++i) {
        gs.activePlayer = i;
        const auto color = SC(gs.staticGs.playerColors.at(i));
        awardResources(landTypeBonuses[color].resources, gs);
    }

    gs.activePlayer = 0;
    if (gs.staticGs.playerRaces[0] != Race::Monks) {
        auto minePos = bots_[0]->choosePlaceToBuildForFree(gs, Building::Mine, true);
        build(minePos, Building::Mine, false, gs);
    }

    gs.activePlayer = 1;
    if (gs.staticGs.playerRaces[1] != Race::Monks) {
        auto minePos = bots_[1]->choosePlaceToBuildForFree(gs, Building::Mine, true);
        build(minePos, Building::Mine, false, gs);
        minePos = bots_[1]->choosePlaceToBuildForFree(gs, Building::Mine, true);
        build(minePos, Building::Mine, false, gs);
    }

    gs.activePlayer = 0;
    if (gs.staticGs.playerRaces[0] != Race::Monks) {
        auto minePos = bots_[0]->choosePlaceToBuildForFree(gs, Building::Mine, true);
        build(minePos, Building::Mine, false, gs);
    }

    for (int i = 0; i < 2; ++i) {
        gs.activePlayer = i;

        if (gs.staticGs.playerRaces.at(i) == Race::Monks) {
            const auto academyPos = bots_[i]->choosePlaceToBuildForFree(gs, Building::Academy, false);
            build(academyPos, Building::Academy, false, gs);
        }

        if (gs.staticGs.playerRaces.at(i) == Race::Inventors || gs.staticGs.playerRaces.at(i) == Race::Monks) {
            const auto tileIdx = bots_[i]->chooseTechTile(gs);
            awardTechTile(tileIdx, gs);
        }
        if (gs.staticGs.playerRaces.at(i) == Race::Omar) {
            const auto towerPos = bots_[i]->choosePlaceToBuildForFree(gs, Building::Tower, true);
            build(towerPos, Building::Tower, true, gs);
        }
    }

    // Choose boosters
    ...
    gs.activePlayer = 0;
}

std::array<IncomableResources, 7> GameEngine::generateFedTiles()
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

std::array<RoundScoreBonus, 16> GameEngine::generateRoundScoreBonuses()
{
    return std::array<RoundScoreBonus, 16> {
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Laboratory), .bonusWp = 4,
            .god = BookColor::Yellow, .godAmount = 1, .resourceBonus = IncomableResources{ .gold = 1 }},
        RoundScoreBonus{.event = EventType::BuildHuge, .bonusWp = 5,
            .god = BookColor::Yellow, .godAmount = 2, .resourceBonus = IncomableResources{ .cube = 1 }},
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Mine), .bonusWp = 2,
            .god = BookColor::Yellow, .godAmount = 3, .resourceBonus = IncomableResources{ .manaCharge = 4 }},
        RoundScoreBonus{.event = EventType::GetInvention, .bonusWp = 5,
            .god = BookColor::Blue, .godAmount = 2, .resourceBonus = IncomableResources{ .manaCharge = 3 }},
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Mine), .bonusWp = 2,
            .god = BookColor::Blue, .godAmount = 3, .resourceBonus = IncomableResources{ .humans = 1 }},
        RoundScoreBonus{.event = EventType::MoveGod, .bonusWp = 1,
            .god = BookColor::White, .godAmount = 3, .resourceBonus = IncomableResources{ .anyBook = 1 }},
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Guild), .bonusWp = 3,
            .god = BookColor::Blue, .godAmount = 3, .resourceBonus = IncomableResources{ .anyBook = 1 }},
        RoundScoreBonus{.event = EventType::UpgradeNavOrTerra, .bonusWp = 3,
            .god = BookColor::Brown, .godAmount = 3, .resourceBonus = IncomableResources{ .humans = 1 }},
        RoundScoreBonus{.event = EventType::FormFederation, .bonusWp = 5,
            .god = BookColor::Brown, .godAmount = 4, .resourceBonus = IncomableResources{ .spades = 1 }},
        RoundScoreBonus{.event = EventType::BuildHuge, .bonusWp = 5,
            .god = BookColor::White, .godAmount = 2, .resourceBonus = IncomableResources{ .cube = 1 }},
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Guild), .bonusWp = 3,
            .god = BookColor::White, .godAmount = 4, .resourceBonus = IncomableResources{ .spades = 1 }},
        
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Mine), .bonusWp = 2},
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Laboratory), .bonusWp = 4},
        RoundScoreBonus{.event = EventType::BuildExactType, .eventParams = SC(Building::Guild), .bonusWp = 3},
        RoundScoreBonus{.event = EventType::BuildOnEdge, .bonusWp = 3},
    };
}

std::array<RoundBoosterOrigin, 10> GameEngine::generateRoundBooosters()
{
    return std::array<RoundBoosterOrigin, 10> {
        RoundBoosterOrigin{.navBooster = true},
        RoundBoosterOrigin{.resources = IncomableResources{.humans = 1}, .trigger = EventType::PutManToGod, .wpPerTrigger = 2},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .action = ActionType::TerraformAndBuild},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .action = ActionType::Bridge},
        RoundBoosterOrigin{.resources = IncomableResources{.anyBook = 1}, .scoreHuge = true},
        RoundBoosterOrigin{.resources = IncomableResources{.cube = 2}, .action = ActionType::MoveOnGods},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 2, .manaCharge = 4}},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 6}},
        RoundBoosterOrigin{.resources = IncomableResources{.manaCharge = 3}, .scoreGuilds = true},
        RoundBoosterOrigin{.resources = IncomableResources{.gold = 4}, .godsForLabs = true},
    };
}

std::array<RaceStartBonus, 12> GameEngine::generateRaceStartBonus() {
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

std::array<Palace, 17> GameEngine::generatePalaces() {
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

std::array<BookAction, 6> GameEngine::generateBookActions() {
    return std::array<BookAction, 6> {
        BookAction { .price = 1, .resources = IncomableResources { .manaCharge = 5 }, .special = BookActionSpecial::None},
        BookAction { .price = 1, .resources = IncomableResources { .anyGod = 2 }, .special = BookActionSpecial::None},
        BookAction { .price = 2, .resources = IncomableResources { .gold = 6 }, .special = BookActionSpecial::None},
        BookAction { .price = 2, .resources = IncomableResources {}, .special = BookActionSpecial::UpgradeMine},
        BookAction { .price = 2, .resources = IncomableResources {}, .special = BookActionSpecial::Guild2wp},
        BookAction { .price = 3, .resources = IncomableResources { .spades = 3 }, .special = BookActionSpecial::None},
    };
};

std::array<MarketAction, 6> GameEngine::generateMarketActions() {
    return std::array<MarketAction, 6> {
        MarketAction { .price = 3, .resources = IncomableResources { }, .bridge = true },
        MarketAction { .price = 3, .resources = IncomableResources { .humans = 1 } },
        MarketAction { .price = 4, .resources = IncomableResources { .cube = 2 } },
        MarketAction { .price = 4, .resources = IncomableResources { .gold = 7 } },
        MarketAction { .price = 4, .resources = IncomableResources { .spades = 1 } },
        MarketAction { .price = 6, .resources = IncomableResources { .spades = 2 } },
    };
}

std::array<LandTypeBonus, 7> GameEngine::generateLandTypeBonuses() {
    return std::array<LandTypeBonus, 7> {
        LandTypeBonus {.resources = IncomableResources { .spades = 1 }, .special = LandTypeBonusSpecial::None },
        LandTypeBonus {.resources = IncomableResources {}, .special = LandTypeBonusSpecial::TerraformCheap },
        LandTypeBonus {.resources = IncomableResources { .humans = 1, .manaCharge = 2 }, .special = LandTypeBonusSpecial::None },
        LandTypeBonus {.resources = IncomableResources {}, .special = LandTypeBonusSpecial::Nav },
        LandTypeBonus {.resources = IncomableResources { .humans = 1, .anyGod = -20 }, .special = LandTypeBonusSpecial::None },
        LandTypeBonus {.resources = IncomableResources {}, .special = LandTypeBonusSpecial::IncomeBetter },
        LandTypeBonus {.resources = IncomableResources { .cube = 1, .anyBook = 1 }, .special = LandTypeBonusSpecial::NoBook2ndInvention },
    };
};

std::array<FieldHex, 13> GameEngine::generateFieldHexes()
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

std::vector<ResizableArray<uint16_t, 6>> GameEngine::generateFieldTopology(int mapSize)
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
