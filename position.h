#pragma once

#include <iostream>
#include <list>
#include <set>

#include "utils.h"
#include "hashed.h"

struct Position
{
    Position(const int& x=-1, const int& y=-1);

    void
    with_direction(const Direction& direction);

    bool
    next_to(const Position& position) const;

    int x;
    int y;

};

Hash
hash_value(const Position& position);

std::ostream&
operator<<(std::ostream& os, const Position& position);

bool
operator==(const Position& position_aa, const Position& position_bb);

bool
operator<(const Position& position_aa, const Position& position_bb);

bool
operator!=(const Position& position_aa, const Position& position_bb);

typedef std::set<Position> PositionsSet;

