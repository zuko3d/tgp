#include "MctsBot.h"

#include "Timer.h"
#include "Types.h"

struct MctsNode {
    const MctsNode* findBestChild() const {
        assert (!children.empty());

        const MctsNode *bestChild = &children.front();
        double bestPts  = bestChild->bestPerspectivePts;
        for (const auto& child: children) {
            if (child.bestPerspectivePts > bestPts) {
                bestPts = child.bestPerspectivePts;
                bestChild = &child;
            }
        }

        return bestChild;
    }

    GameState gs;
    Action actionToGetHere = Action{};
    MctsNode* parent = nullptr;
    int depth = 0;

    double pts = -123;
    double bestPerspectivePts = -123;
    bool generatedChildren = false;
    std::vector<MctsNode> children;
    std::vector<MctsNode> bottomChildren;
    int stepIns = 0;
    bool nodeIsCompletelyEvaluated = false;
};

double MctsBot::evalAction(const GameState& gs, Action action) const {
    const auto ap = gs.activePlayer;
    const auto& ps = gs.players[gs.activePlayer];

    const auto& res = ps.resources;
    const auto& curWeights = allScoreWeights_[gs.round];

    const auto evalResources = [&curWeights](const IncomableResources& res) {
        return res.anyBook * curWeights.totalBooks +
            res.anyGod * curWeights.godMove +
            res.cube * curWeights.cube +
            res.gold * curWeights.gold +
            res.humans * curWeights.humans +
            res.manaCharge * curWeights.manaCharge +
            res.spades * curWeights.spades +
            res.winPoints * curWeights.winPoints;
    };
    const auto evalButton = [&curWeights, evalResources, &ps](int buttonOriginIdx) {
        const auto& button = StaticData::buttonOrigins()[buttonOriginIdx];
        double ret = 0.0;
        ret += evalResources(button.resources);

        switch (button.special) {
        case ButtonActionSpecial::BuildBridge: {
            break;
        }
        case ButtonActionSpecial::FiraksButton: {
            ret += curWeights.scorePerBuilding[SC(Building::Guild)] - curWeights.scorePerBuilding[SC(Building::Laboratory)];
            break;
        }
        case ButtonActionSpecial::UpgradeMine: {
            ret += curWeights.scorePerBuilding[SC(Building::Guild)] - curWeights.scorePerBuilding[SC(Building::Mine)];
            break;
        }
        case ButtonActionSpecial::WpForGuilds2: {
            ret += 2 * curWeights.winPoints * ps.countBuildings(Building::Guild);
            break;
        }
        case ButtonActionSpecial::None: {
            break;
        }
        };

        return ret;
    };

    switch (action.type) {
    case ActionType::UpgradeBuilding: {
        const auto pos = action.param1;
        Building newType;
        double additionalScore = 0.0;
        
        if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Guild) {
            if (action.param2 >= 0) {
                newType = Building::Palace;
                additionalScore += curWeights.scorePerPalaceIdx[action.param2];
            }
            else {
                newType = Building::Laboratory;
                additionalScore = curWeights.scorePerTech[-1 - action.param2];
            }
        }
        else if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Mine) {
            newType = Building::Guild;
        }
        else if (gs.cache->fieldByState_[gs.fieldStateIdx].building[pos].type == Building::Laboratory) {
            newType = Building::Academy;
            additionalScore = curWeights.scorePerTech[action.param2];
        }

        return curWeights.scorePerBuilding[SC(newType)] - curWeights.scorePerBuilding[SC(gs.field().building[pos].type)];
    }
    case ActionType::TerraformAndBuild: {
        return curWeights.scorePerBuilding[SC(Building::Mine)];
    }
    case ActionType::UpgradeNav: {
        return curWeights.navLevel[ps.navLevel + 1] - curWeights.navLevel[ps.navLevel];
        break;
    }
    case ActionType::UpgradeTerraform: {
        return curWeights.tfLevel[ps.navLevel + 1] - curWeights.tfLevel[ps.navLevel];
        break;
    }
    case ActionType::GetInnovation: {
        return curWeights.scorePerInnovation[action.param1];
    }
    case ActionType::PutManToGod: {
        return curWeights.godMove * action.param2;
    }
    case ActionType::BookMarket: {
        const auto& bookAction = gs.bookActions[action.param1];
        return evalButton(bookAction.buttonOrigin) - curWeights.totalBooks * bookAction.bookPrice;
    }

    case ActionType::ActivateAbility: {
        if (action.param1 < 0) {
            return evalButton(ps.boosterButton.buttonOrigin);
        } else {
            return evalButton(ps.buttons[action.param1].buttonOrigin);
        }
        break;
    }

    case ActionType::Market: {
        auto& marketAction = gs.marketActions[action.param1];
        return evalButton(marketAction.buttonOrigin) - curWeights.manaCharge * 2 * marketAction.manaPrice;
    }

    case ActionType::Annex: {
        return curWeights.totalPower;
    }

    case ActionType::Pass: {
        return 0;
    }
    default:
        assert(false);
    }

    assert(false);
    return -1;
}

double MctsBot::evalPs(const GameState& gs, int pIdx) const {
    const auto ap = gs.activePlayer;
    GameState* hackedGs = const_cast<GameState*>(&gs);
    hackedGs->activePlayer = pIdx;
    const auto& ps = gs.players[pIdx];

    double ret = 0.0;

    const auto& res = ps.resources;
    const auto& curWeights = allScoreWeights_[gs.round];

    ret += res.gold * curWeights.gold;
    ret += res.cube * curWeights.cube;
    ret += res.humans * curWeights.humans;
    ret += sum(res.books.values()) * curWeights.totalBooks;
    ret += sum(res.gods.values()) * curWeights.totalGods;
    ret += res.winPoints * curWeights.winPoints;

    ret += ps.additionalIncome.gold * curWeights.goldIncome;
    ret += ps.additionalIncome.cube * curWeights.cubeIncome;
    ret += ps.additionalIncome.humans * curWeights.humansIncome;
    ret += ps.additionalIncome.anyBook * curWeights.booksIncome;
    ret += ps.additionalIncome.anyGod * curWeights.godsIncome;
    ret += ps.additionalIncome.manaCharge * curWeights.manaIncome;
    
    const auto hexes = ownGe_.someHexes(true, false, gs, 0, 0);
    const auto color = gs.staticGs.playerColors[pIdx];
    std::array<int, 4> tfs = {{0}};
    for (const auto pos: hexes) {
        tfs[spadesNeeded(gs.field().type[pos], color)]++;
    }
    for (int i = 0; i < 4; i++) {
        ret += curWeights.reachableHexes[i] * tfs[i];
    }

    ret += curWeights.navLevel[ps.navLevel];
    ret += curWeights.tfLevel[ps.tfLevel];

    if (gs.round < 5) {
        ret += curWeights.targetGod * res.gods[StaticData::roundScoreBonuses()[gs.staticGs.bonusByRound[gs.round]].god];
    }

    hackedGs->activePlayer = ap;
    return ret;
}

void MctsBot::genChildren(MctsNode& node) const {
    const auto actions = ownGe_.generateActions(node.gs);
    assert(!actions.empty());

    node.children.reserve(actions.size());
    for (const auto& action: actions) {
        GameState newState(node.gs);
        advanceToMyNextState(action, newState);

        assert((newState.activePlayer == node.gs.activePlayer) || ownGe_.gameEnded(newState));

        double pts = evalPs(newState, node.gs.activePlayer);
        node.children.emplace_back(MctsNode{
            .gs = newState,
            .actionToGetHere = action,
            .parent = &node,
            .depth = node.depth + 1,
            .pts = pts,
        });
        if (ownGe_.gameEnded(newState)) {
            node.children.back().nodeIsCompletelyEvaluated = true;
            node.children.back().generatedChildren = true;
            node.children.back().bestPerspectivePts = pts;
        }
    }

    node.generatedChildren = true;
}

void MctsBot::advanceToMyNextState(const Action& action, GameState& gs) const {
    const auto myPlayer = gs.activePlayer;

    ownGe_.doAction(action, gs);
    ownGe_.doAfterTurnActions(gs);
    while (!ownGe_.gameEnded(gs) && (myPlayer != gs.activePlayer)) {
        ownGe_.advanceGs(gs);
    }
}

void MctsBot::descentToFloor(GameState& gs, int stopRound) const {
    while((gs.round < stopRound) && !ownGe_.gameEnded(gs)) {
        const auto actions = ownGe_.generateActions(gs);
        assert(!actions.empty());
        Action bestAction;
        double bestPts = -1e9;
        for (const auto action : actions) {
            double pts = evalAction(gs, action);

            if (bestPts < pts) {
                bestPts = pts;
                bestAction = action;
            }
        }

        advanceToMyNextState(bestAction, gs);
    }
}

MctsNode* MctsBot::goBottom(MctsNode& node, int stopRound) const {
    MctsNode* curNode = &node;

    while (curNode != nullptr) {
        curNode->stepIns++;
        if ((curNode->gs.round >= stopRound) || ownGe_.gameEnded(curNode->gs) || curNode->nodeIsCompletelyEvaluated) {
            break;
        }

        if (curNode->depth >= maxDepth_) {
            auto newGs = curNode->gs;
            // std::cout << node.gs.cache->fieldByState_.size() << " -> ";
            descentToFloor(newGs, stopRound);
            // std::cout << node.gs.cache->fieldByState_.size() << std::endl;
            curNode->bottomChildren.push_back(MctsNode{
                .gs = newGs,
                .actionToGetHere = {},
                .parent = curNode,
                .depth = curNode->depth + 1,
                .pts = evalPs(newGs, curNode->gs.activePlayer)
            });
            curNode->bottomChildren.back().bestPerspectivePts = curNode->bottomChildren.back().pts;
            return &curNode->bottomChildren.back();
        }

        if (!curNode->generatedChildren) {
            genChildren(*curNode);
        }

        // Need exploration?
        std::vector<MctsNode*> explorationCandidates;
        explorationCandidates.reserve(curNode->children.size());
        for (auto& child: curNode->children) {
            if (child.stepIns == 0) {
                explorationCandidates.push_back(&child);
            }
        }
        if (!explorationCandidates.empty()) {
            double bestPts  =-1e9;
            MctsNode *bestChild = explorationCandidates.front();
            for (auto& child: explorationCandidates) {
                if (child->pts > bestPts) {
                    bestPts = child->pts;
                    bestChild = child;
                }
            }

            curNode = bestChild;
        } else {
            double bestPts  =-1e9;
            assert(!curNode->children.empty());
            MctsNode *bestChild = &(curNode->children.front());
            double minPts = curNode->children.front().bestPerspectivePts;
            double maxPts = curNode->children.front().bestPerspectivePts;
            for (auto& child: curNode->children) {
                minPts = std::min(child.bestPerspectivePts, minPts);
                maxPts = std::max(child.bestPerspectivePts, maxPts);
            }
            double deltaPts = maxPts - minPts + 1e-6;

            for (auto& child: curNode->children) {
                if (child.nodeIsCompletelyEvaluated) {
                    continue;
                }

                double mctsPoints = (child.bestPerspectivePts - minPts) / deltaPts + 1.4 * sqrt(log(curNode->stepIns) / (child.stepIns + 1e-6));

                if (mctsPoints > bestPts) {
                    bestPts = mctsPoints;
                    bestChild = &child;
                }
            }

            curNode = bestChild;
        }
    }

    return curNode;
}

bool allChildrenAreCalculated(const MctsNode& node) {
    for (const auto& child: node.children) {
        if (!child.nodeIsCompletelyEvaluated) {
            return false;
        }
    }

    return true;
}

Action MctsBot::buildMcTree(const GameState& gs) const {
    MctsNode root { .gs = gs };
    double bottomPts = -123.0;

    int stopRound = gs.round + roundsDepth_;
    for (int step = 0; step < steps_; step++) {
        Timer timer;

        // std::cout << "step: " << step << ", fs: " << gs.cache->fieldByState_.size() << std::endl;
        MctsNode* bottom = goBottom(root, stopRound);
        bottomPts = bottom->pts;

        while (bottom->parent != nullptr) {
            bottom->parent->bestPerspectivePts = std::max(bottom->parent->bestPerspectivePts, bottomPts);
            bottom->nodeIsCompletelyEvaluated = allChildrenAreCalculated(*bottom);

            bottom = bottom->parent;
        }

//         std::cout << timer.elapsedUSeconds() << std::endl;
    }

    return root.findBestChild()->actionToGetHere;
}
