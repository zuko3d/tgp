#pragma once

#include "Resources.h"

enum class SpecialAction {
    BuildBridge,
    Terraform1,
    Terraform2,
    Terraform3,
    UpgradeMineToGuild
};

struct ActivedAbility
{
    int manaPrice = 0;
    IncomableResources effect;
    int freeTerraforms = 0;
    bool used = false;
};
