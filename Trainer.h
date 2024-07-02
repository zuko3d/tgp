#pragma once

#include "GameState.h"

class Trainer {
public:
    void train(AllScoreWeights& weights1, int epochs);

    void train2(AllScoreWeights& weights, int epochs);
};