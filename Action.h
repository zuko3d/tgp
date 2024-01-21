#pragma once

enum class ActionType {
    Build,
    UpgradeBuilding,
    Market,
    ActivateAbility,
    PutManToGod,
    MoveOnGods,
    GetInnovation,
    TerraformAndBuild,
    TerraformOnly,
    Bridge,
    Annex,
    Pass,
};

struct Action
{
    ActionType type;
    int param1, param2;
};

