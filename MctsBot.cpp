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
    int stepIns = 0;
    bool nodeIsCompletelyEvaluated = false;
};

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
        advanceToNextNode(action, newState);

        assert((newState.activePlayer == node.gs.activePlayer) || ownGe_.gameEnded(newState));

        double pts = evalPs(newState, node.gs.activePlayer);
        node.children.emplace_back(MctsNode{
            .gs = newState,
            .actionToGetHere = action,
            .parent = &node,
            .depth = node.depth + 1,
            .pts = pts,
    // double bestPerspectivePts = -123;
    // bool generatedChildren = false;
    // std::vector<MctsNode> children;
    // int stepIns = 0;
    // bool nodeIsCompletelyEvaluated = false;
        });
        if (ownGe_.gameEnded(newState)) {
            node.children.back().nodeIsCompletelyEvaluated = true;
            node.children.back().generatedChildren = true;
            node.children.back().bestPerspectivePts = pts;
        }
    }

    node.generatedChildren = true;
}

void MctsBot::advanceToNextNode(const Action& action, GameState& gs) const {
    const auto myPlayer = gs.activePlayer;

    ownGe_.doAction(action, gs);
    ownGe_.doAfterTurnActions(gs);
    while (!ownGe_.gameEnded(gs) && (myPlayer != gs.activePlayer)) {
        ownGe_.advanceGs(gs);
    }
}

MctsNode* MctsBot::goBottom(MctsNode& node, int stopRound) const {
    MctsNode* curNode = &node;

    while (curNode != nullptr) {
        curNode->stepIns++;
        if ((curNode->depth >= maxDepth_) || (curNode->gs.round >= stopRound) || ownGe_.gameEnded(curNode->gs) || curNode->nodeIsCompletelyEvaluated) {
            break;
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
