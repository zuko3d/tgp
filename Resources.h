#pragma once

#include <cstdint>

#include "FlatMap.h"

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

    void operator+=(const IncomableResources& op) {
        int8_t* ptr1 = (int8_t*) this;
        const int8_t* ptr2 = (int8_t*) &op;
        for (int i = 0; i < sizeof(*this); i++) {
            *ptr1 += *ptr2;
            ++ptr1;
            ++ptr2;
        }
    }

    void operator-=(const IncomableResources& op) {
        int8_t* ptr1 = (int8_t*) this;
        const int8_t* ptr2 = (int8_t*) &op;
        for (int i = 0; i < sizeof(*this); i++) {
            *ptr1 -= *ptr2;
            ++ptr1;
            ++ptr2;
        }
    }
};

struct Resources
{
    int8_t gold = 0;
    int8_t cube = 0;
    int8_t humans = 0;
    FlatMap<GodColor, int8_t, 4> gods = {};
    FlatMap<BookColor, int8_t, 4> books = {};
    int16_t winPoints = {};

    void operator+=(const Resources& op) {
        gold += op.gold;
        cube += op.cube;
        humans += op.humans;
        for (size_t i = 0; i < books.size(); i++) {
            books[(BookColor) i] += op.books[(BookColor) i];
        }
        for (size_t i = 0; i < gods.size(); i++) {
            gods[(GodColor) i] = std::min(gods[(GodColor) i] + op.gods[(GodColor) i], 12);
        }
        winPoints += op.winPoints;
    }

    bool operator >= (const IncomableResources& op) {
        return gold >= op.gold &&
            cube >= op.cube;
    }

    bool operator >= (const InnoPrice& op) {
        if (op.gold > gold) return false;

        FlatMap<BookColor, int8_t, 4> booksLeft = books;
        for (auto& [color, val]: booksLeft) {
            val -= books[color];
        }

        return sum(booksLeft.values()) >= op.anyBooks;
    }
};
