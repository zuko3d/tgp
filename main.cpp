#include "GameEngine.h"
#include "RandomBot.h"

#include <random>
#include <iostream>

int main() {
    std::default_random_engine g{42};
    auto bot1 = RandomBot(std::default_random_engine{1});
    auto bot2 = RandomBot(std::default_random_engine{2});

    StaticGameState sgs;
    GameState gs { .staticGs = sgs };

    GameEngine ge({ &bot1, &bot2 });

    ge.initializeRandomly(gs, g);
    ge.playGame(gs);

    for (const auto& p: gs.players) {
        std::cout << p.resources.winPoints << std::endl;
    }

    return 0;
}