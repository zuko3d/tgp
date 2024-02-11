#pragma once

#include "json.hpp"

#include "Field.h"
#include "FlatMap.h"
#include "ResizableArray.h"
#include "Types.h"
#include "Utils.h"
#include "Player.h"
#include "GameState.h"

#include <array>
#include <string>
#include <vector>

int toJson(int8_t v);

nlohmann::json toJson(const RoundBoosterOnBoard& op);

nlohmann::json toJson(const RoundScoreBonus& op);

nlohmann::json toJson(const IncomableResources& res);

nlohmann::json toJson(const Button& res);

nlohmann::json toJson(const Resources& res);

nlohmann::json toJson(const Resources& res);

nlohmann::json toJson(const FederationTile& op);

nlohmann::json toJson(const BuildingOnMap& op);

nlohmann::json toJson(const Palace& op);

nlohmann::json toJson(const BookButton& op);

nlohmann::json toJson(const MarketButton& op);

nlohmann::json toJson(const TechTile& op);

nlohmann::json toJson(const TerrainType& op);

nlohmann::json toJson(const Race& op);

nlohmann::json toJson(const Innovation& op);

nlohmann::json toJson(const PlayerState& ps);

nlohmann::json toJson(const Field& f);

nlohmann::json toJson(const StaticGameState& sgs);

nlohmann::json toJson(const GameState& gs);

template <typename KeyType, typename ValueType, size_t Size>
inline nlohmann::json toJson(const FlatMap<KeyType, ValueType, Size>& mp) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& v : mp.values()) {
        j.push_back(toJson(v));
    }

    return j;
}

template <typename T, size_t N>
inline nlohmann::json toJson(const std::array<T, N>& arr) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& v : arr) {
        j.push_back(toJson(v));
    }

    return j;
}

template <typename T>
inline nlohmann::json toJson(const std::vector<T>& arr) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& v : arr) {
        j.push_back(toJson(v));
    }

    return j;
}

template <typename T, size_t N>
inline nlohmann::json toJson(const ResizableArray<T, N>& arr) {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& v : arr) {
        j.push_back(toJson(v));
    }

    return j;
}
