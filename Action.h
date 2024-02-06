#pragma once

#include <vector>

enum class ActionType {
    // Build,
    UpgradeBuilding,
    Market,
    BookMarket,
    ActivateAbility,
    PutManToGod,
    // MoveOnGods,
    GetInnovation,
    TerraformAndBuild,
    UpgradeNav,
    UpgradeTerraform,
    // TerraformOnly,
    Bridge,
    Annex,
    Pass,
};

enum class FreeActionMarketType {
    ManaToHuman,
    ManaToCube,
    ManaToGold,
    ManaToBook,
    HumanToCube,
    CubeToGold,
    BookToGold,
    BurnMana,
};

// struct FreeActionMarket {
//     IncomableResources resources;
//     Resources price;
// };

struct Action
{
    ActionType type = ActionType::Pass;
    int param1, param2;
};

struct FullAction {
    std::vector<FreeActionMarketType> preAction;
    Action action;
    std::vector<FreeActionMarketType> postAction;
};
