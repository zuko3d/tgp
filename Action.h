#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <assert.h>

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
    Bridge, // 9, unused
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

    std::string toString() const { 
        std::ostringstream ret;

        switch (type) {
            case ActionType::UpgradeBuilding:
                ret << "UpgradeBuilding";
                break;
            case ActionType::Market:
                ret << "Market";
                break;
            case ActionType::BookMarket:
                ret << "BookMarket";
                break;
            case ActionType::ActivateAbility:
                ret << "ActivateAbility";
                break;
            case ActionType::PutManToGod:
                ret << "PutManToGod";
                break;
            case ActionType::GetInnovation:
                ret << "GetInnovation";
                break;
            case ActionType::TerraformAndBuild:
                ret << "TerraformAndBuild";
                break;
            case ActionType::UpgradeNav:
                ret << "UpgradeNav";
                break;
            case ActionType::UpgradeTerraform:
                ret << "UpgradeTerraform";
                break;
            case ActionType::Bridge:
                ret << "Bridge";
                break;
            case ActionType::Annex:
                ret << "Annex";
                break;
            case ActionType::Pass:
                ret << "Pass";
                break;
            default:
                assert(false);
                return "Unknown";
        }

        return ret.str();
    }
};

struct FullAction {
    std::vector<FreeActionMarketType> preAction;
    Action action;
    std::vector<FreeActionMarketType> postAction;
};
