#pragma once

#include "GameState.h"

#include <string>
#include <vector>

struct GameInfo
{
    GameState gs;
    Field field;

    std::vector<std::string> logs;
};

using GameHistory = std::vector<GameInfo>;
