#include "position.h"

#include <boost/functional/hash.hpp>

Position::Position(const int& x, const int& y) :
    x(x), y(y)
{
}

void
Position::with_direction(const Direction& direction)
{
    switch (direction)
    {
    case STAY:
        return;
    case NORTH:
        x--;
        return;
    case SOUTH:
        x++;
        return;
    case EAST:
        y++;
        return;
    case WEST:
        y--;
        return;
    }

    /*
    static const Position deltas[5] = {
        Position(0,0),   // STAY
        Position(-1,0),  // NORTH
        Position(1,0),   // SOUTH
        Position(0,1),   // EAST
        Position(0,-1),  // WEST
    };
    */
}

bool
Position::next_to(const Position& position) const
{
    const int& delta_x = x-position.x;
    if (delta_x > 1 || delta_x < -1) return false;
    const int& delta_y = y-position.y;
    if (delta_y > 1 || delta_y < -1) return false;
    if (delta_x == 0) return true;
    if (delta_y == 0) return true;
    return false;
}

Hash
hash_value(const Position& position)
{
    Hash seed = 42;
    boost::hash_combine(seed, position.x);
    boost::hash_combine(seed, position.y);
    return seed;
}

std::ostream&
operator<<(std::ostream& os, const Position& position)
{
    os << "[" << position.x << "," << position.y << "]";
    return os;
}

bool
operator==(const Position& position_aa, const Position& position_bb)
{
    return position_aa.x == position_bb.x && position_aa.y == position_bb.y;
}

bool
operator!=(const Position& position_aa, const Position& position_bb)
{
    return position_aa.x != position_bb.x || position_aa.y != position_bb.y;
}

bool
operator<(const Position& position_aa, const Position& position_bb)
{
    if (position_aa.x != position_bb.x) return position_aa.x < position_bb.x;
    return position_aa.y < position_bb.y;
}

