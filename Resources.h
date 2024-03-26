#pragma once

#include <cstdint>
#include <sstream>

#include "FlatMap.h"
#include "Utils.h"

enum class BookColor : uint8_t {
    Yellow = 0,
    Blue = 1,
    Brown = 2,
    White = 3,
};

using GodColor = BookColor;

struct InnoPrice {
    FlatMap<BookColor, int8_t, 4> books;
    int8_t anyBooks = 0;
    int8_t gold = 0;
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

    std::string toHtmlString() const {
        std::ostringstream ret;
        for (int i = 0; i < gold; i++) {
            ret << "<img src=\"pics/gold_1.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < cube; i++) {
            ret << "<img src=\"pics/cube.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < humans; i++) {
            ret << "<img src=\"pics/human.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < anyGod; i++) {
            ret << "<img src=\"pics/anygod.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < anyBook; i++) {
            ret << "<img src=\"pics/anybook.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < manaCharge; i++) {
            ret << "<img src=\"pics/mana.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < spades; i++) {
            ret << "<img src=\"pics/spade.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < winPoints; i++) {
            ret << "<img src=\"pics/wp.png\" style=\"height: 26;\">";
        }
        return ret.str();
    }
};

struct Resources
{
    int8_t gold = 0;
    int8_t cube = 0;
    int8_t humans = 0;
    FlatMap<GodColor, int8_t, 4> gods = {0, 0, 0, 0};
    FlatMap<BookColor, int8_t, 4> books = {0, 0, 0, 0};
    int16_t winPoints = 0;

    void operator+=(const Resources& op) {
        gold += op.gold;
        cube += op.cube;
        humans += op.humans;
        for (size_t i = 0; i < 4; i++) {
            books[(BookColor) i] += op.books[(BookColor) i];
        }
        for (size_t i = 0; i < 4; i++) {
            assert(op.gods[(GodColor) i] == 0); // Use moveGod(val, color, gs);
        }
        winPoints += op.winPoints;
    }

    bool operator >= (const Resources& op) const {
        if (gold < op.gold) return false;
        if (cube < op.cube) return false;
        if (humans < op.humans) return false;
        for (size_t i = 0; i < 4; i++) {
            if (books[(BookColor) i] < op.books[(BookColor) i]) return false;
        }

        return true;
    }

    bool operator >= (const IncomableResources& op) const {
        return gold >= op.gold &&
            cube >= op.cube;
    }

    bool operator >= (const InnoPrice& op) const {
        if (op.gold > gold) return false;

        FlatMap<BookColor, int8_t, 4> booksLeft = books;
        for (const auto [color, _]: booksLeft) {
            booksLeft[color] -= op.books[color];
            if (booksLeft[color] < 0) return false;
        }

        return sum(booksLeft.values()) >= op.anyBooks;
    }

    std::string toHtmlString() const {
        std::ostringstream ret;
        for (int i = 0; i < gold; i++) {
            ret << "<img src=\"pics/gold_1.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < cube; i++) {
            ret << "<img src=\"pics/cube.png\" style=\"height: 26;\">";
        }
        for (int i = 0; i < humans; i++) {
            ret << "<img src=\"pics/human.png\" style=\"height: 26;\">";
        }

        for (const auto [book, amount] : books) {
            for (int i = 0; i < amount; i++) {
                ret << "<img src=\"pics/book_" << SC(book) << ".png\" style=\"height: 26;\">";
            }
        }
        for (const auto [god, amount] : gods) {
            for (int i = 0; i < amount; i++) {
                ret << "<img src=\"pics/god_" << SC(god) << ".png\" style=\"height: 26;\">";
            }
        }

        for (int i = 0; i < winPoints; i++) {
            ret << "<img src=\"pics/wp.png\" style=\"height: 26;\">";
        }
        return ret.str();
    }
};

inline FlatMap<BookColor, int8_t, 4> genBook(const BookColor book, int8_t amount) {
    FlatMap<BookColor, int8_t, 4> ret;
    ret[book] = amount;
    return ret;
}
