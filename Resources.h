#pragma once

#include <cstdint>

struct Resources
{
    int8_t gold;
    int8_t cube;
    int8_t humans;
    std::array<int8_t, 4> gods;
    std::array<int8_t, 4> books;
    int16_t winPoints;

    void operator+=(const Resources& op) {
        gold += op.gold;
        cube += op.cube;
        humans += op.humans;
        for (size_t i = 0; i < books.size(); i++) {
            books[i] += op.books[i];
        }
        for (size_t i = 0; i < gods.size(); i++) {
            gods[i] = std::min(gods[i] + op.gods[i], 12);
        }
        winPoints += op.winPoints;
    }
};

struct IncomableResources
{
    int8_t gold = 0;
    int8_t cube = 0;
    int8_t humans = 0;
    int8_t anyGod = 0;
    int8_t anyBook = 0;
    
    int8_t manaCharge = 0;

    int8_t spades = 0;
    int8_t winPoints = 0;
};
