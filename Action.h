#pragma once

#include <vector>

enum class ActionType {
    UpgradeBuilding, // 0
    Market, // 1
    BookMarket, // 2
    ActivateAbility, // 3
    PutManToGod, // 4
    GetInnovation, // 5
    TerraformAndBuild, // 6
    UpgradeNav, // 7
    UpgradeTerraform, // 8
    Bridge, // 9
    Annex, // 10
    Pass, // 11
    None, // 12
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
