#include "tiles.h"

#include <cassert>
#include <boost/functional/hash.hpp>

Tiles
parse_tiles(const int& tiles_size, const std::string& tiles_string)
{
    assert(tiles_size >= 0);

    Tiles tiles(boost::extents[tiles_size][tiles_size]);

    std::stringstream tiles_stream(tiles_string);
    Tile* flat = tiles.data();

    for (int kk=0; kk<tiles_size*tiles_size; kk++)
    {
        Tile tile;
        tiles_stream >> tile;
        assert( tile != UNKNOWN );
        flat[kk] = tile;
    }

    return tiles;
}

Hash
hash_value(const Tiles& tiles)
{

    const Tile* data = tiles.origin();
    Hash seed = boost::hash_range(data, data+tiles.num_elements());

    const size_t* shape = tiles.shape();
    const size_t& dim = tiles.num_dimensions();
    boost::hash_combine(seed, dim);
    for (size_t kk=0; kk<dim; kk++)
        boost::hash_combine(seed, shape[kk]);

    return seed;
}

Tiles
neutralize_tiles(const Tiles& tiles_orig)
{
    Tiles tiles(tiles_orig);

    Tile* data = tiles.origin();
    for (size_t kk=0, kk_max=tiles.num_elements(); kk<kk_max; kk++)
    {
        Tile &tile = data[kk];
        switch (tile)
        {
        case UNKNOWN:
        case EMPTY:
        case WOOD:
        case TAVERN:
        case MINE:
            break;
        case MINE1:
        case MINE2:
        case MINE3:
        case MINE4:
            tile = MINE;
            break;
        case HERO1:
        case HERO2:
        case HERO3:
        case HERO4:
            tile = EMPTY;
            break;
        }
    }

    return tiles;
}

OwnedMines
extract_owned_mines(const Tiles& tiles)
{
    const size_t* shape = tiles.shape();

    OwnedMines owned_mines;
    for (size_t ii=0; ii<shape[0]; ii++)
        for (size_t jj=0; jj<shape[1]; jj++)
        {
            const Tile& tile = tiles[ii][jj];
            const Position position(ii,jj);
            if (tile == MINE1) {
                owned_mines[0].insert(position);
                continue;
            }
            if (tile == MINE2) {
                owned_mines[1].insert(position);
                continue;
            }
            if (tile == MINE3) {
                owned_mines[2].insert(position);
                continue;
            }
            if (tile == MINE4) {
                owned_mines[3].insert(position);
                continue;
            }
        }

    return owned_mines;
}

Tile&
get_tile(Tiles& tiles, const Position& position)
{
    return tiles[position.x][position.y];
}

const Tile&
get_tile(const Tiles& tiles, const Position& position)
{
    return tiles[position.x][position.y];
}

Tile
get_tile_border_check(const Tiles& tiles, const Position& position)
{
    if (position.x < 0) return UNKNOWN;
    if (position.y < 0) return UNKNOWN;

    const size_t* shape = tiles.shape();
    if (position.x >= static_cast<int>(shape[0])) return UNKNOWN;
    if (position.y >= static_cast<int>(shape[1])) return UNKNOWN;

    return get_tile(tiles, position);
}

std::ostream&
operator<<(std::ostream& os, const Tiles& tiles)
{
    const size_t* shape = tiles.shape();

    os << "╔";
    for (size_t jj=0; jj<shape[1]; jj++) os << "══";
    os << "╗" << std::endl;

    for (size_t ii=0; ii<shape[0]; ii++)
    {
        os << "║";
        for (size_t jj=0; jj<shape[1]; jj++) os << tiles[ii][jj];
        os << "║" << std::endl;
    }

    os << "╚";
    for (size_t jj=0; jj<shape[1]; jj++) os << "══";
    os << "╝" << std::endl;

    return os;
}

static const std::string pretty_tile_names[13] = {
    "\033[37m··\033[0m",
    "  ", "##",
    "\033[31m@1\033[0m", "\033[34m@2\033[0m", "\033[32m@3\033[0m", "\033[33m@4\033[0m",
    "[]",
    "$-",
    "$1", "$2", "$3", "$4"
};

static const std::string tile_names[13] = {
    "??",
    "  ", "##",
    "@1", "@2", "@3", "@4",
    "[]",
    "$-",
    "$1", "$2", "$3", "$4"
};

std::ostream&
operator<<(std::ostream& os, const Tile& tile)
{
    return os << pretty_tile_names[static_cast<int>(tile)];
}

std::istream&
operator>>(std::istream& is, Tile& tile)
{
    char buffer[3];
    is.get(buffer, 3);

    for (int tile_int=0; tile_int<13; tile_int++)
        if (tile_names[tile_int] == buffer)
        {
            tile = static_cast<Tile>(tile_int);
            return is;
        }
    std::cout << "UNKNOWN TILE: " << buffer << std::endl;

    tile = UNKNOWN;
    return is;
}
