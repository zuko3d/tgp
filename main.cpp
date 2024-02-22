#include "GameEngine.h"
#include "RandomBot.h"
#include "WebUiBot.h"

#include "serialize.h"

#include <random>
#include <iostream>

int main() {
    std::default_random_engine g{42};
    auto bot1 = WebUiBot(std::default_random_engine{1});
    auto bot2 = RandomBot(std::default_random_engine{2});

    StaticGameState sgs;
    GameState gs { .staticGs = sgs };

    GameEngine ge({ &bot1, &bot2 });

    ge.initializeRandomly(gs, g);
    
    gs.activePlayer = 0;
    bot1.chooseAction(gs, {
        Action{
            .type = ActionType::Market,
            .param1 = 0
        },
        Action{
            .type = ActionType::Market,
            .param1 = 1
        },
        Action{
            .type = ActionType::Market,
            .param1 = 3
        },
        Action{
            .type = ActionType::Market,
            .param1 = 4
        },
        Action{
            .type = ActionType::Market,
            .param1 = 5
        },
    });

    ge.playGame(gs);

    for (const auto& p: gs.players) {
        std::cout << p.resources.winPoints << std::endl;
    }

    // std::cout << toJson(gs) << std::endl;

    return 0;
}