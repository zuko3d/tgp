#pragma once

#include "Field.h"
#include "FlatMap.h"
#include "Types.h"

#include <vector>

class StaticData {
public:
    static const std::array<IncomableResources, 7>& fedTiles();
    static const std::array<RoundBoosterOrigin, 10>& roundBoosters();
    static const std::vector<ButtonOrigin>& buttonOrigins();
    static const FieldOrigin& fieldOrigin();
    static const std::array<Palace, 17>& palaces();
    static const FlatMap<Building, BuildingOrigin, 7>& buildingOrigins();
    static const std::array<InnoPrice, 6>& innoPrices();
    
    static std::array<BookButton, 6> generateBookActions();
    static std::array<MarketButton, 6> generateMarketActions();
    static std::array<RoundScoreBonus, 16> generateRoundScoreBonuses();
    static std::array<RaceStartBonus, 12> generateRaceStartBonus();
    static std::array<LandTypeBonus, 7> generateLandTypeBonuses();

private:
    static std::array<InnoPrice, 6> generateInnoPrices();
    static FlatMap<Building, BuildingOrigin, 7> generateBuildingOrigins();
    static std::vector<ButtonOrigin> generateButtonOrigins();
    static std::array<IncomableResources, 7> generateFedTiles();
    static std::array<RoundBoosterOrigin, 10> generateRoundBoosters();
    static FieldOrigin generateFieldOrigin();
    static std::array<Palace, 17> generatePalaces();
};
