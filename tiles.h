#pragma once

#include "hashed.h"
#include "position.h"
#include <boost/multi_array.hpp>
#include <boost/array.hpp>

enum Tile
{
    UNKNOWN,
    EMPTY,
    WOOD,
    HERO1,
    HERO2,
    HERO3,
    HERO4,
    TAVERN,
    MINE,
    MINE1,
    MINE2,
    MINE3,
    MINE4
};

std::ostream&
operator<<(std::ostream& os, const Tile& tile);

std::istream&
operator>>(std::istream& is, Tile& tile);

typedef boost::multi_array<Tile, 2> Tiles;

Tiles
parse_tiles(const int& tiles_size, const std::string& tiles_string);

Tiles
neutralize_tiles(const Tiles& tiles);

Hash
hash_value(const Tiles& tiles);

Tile&
get_tile(Tiles& tiles, const Position& position);

const Tile&
get_tile(const Tiles& tiles, const Position& position);

Tile
get_tile_border_check(const Tiles& tiles, const Position& position);

typedef boost::array<PositionsSet, 4> OwnedMines;

OwnedMines
extract_owned_mines(const Tiles& tiles);

std::ostream&
operator<<(std::ostream& os, const Tiles& tiles);

