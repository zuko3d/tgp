#pragma once

#include "Types.h"

#include <memory>

// Don't forget to make it Copy-on-write
class Field {
public:
    Field();

    TerrainType terrainType(int8_t pos) const;

private:
    static constexpr size_t FIELD_SIZE = 89;

    std::array<HexType, FIELD_SIZE> type_;
    std::array<bool, FIELD_SIZE> onEdge_;
    std::array<Building, FIELD_SIZE> building_;
    std::array<uint8_t, FIELD_SIZE> owner_;
};
