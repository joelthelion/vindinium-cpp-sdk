#pragma once

#include "hashed.h"
#include "network.h"
#include <boost/array.hpp>

struct State
{
    struct Hero
    {
        Hero();
        Hero(const PTree& root, const PositionsSet& mine_positions);

        void update(const PTree&, const PositionsSet& mine_positions);

        Position position;
        int life;
        int gold;
        PositionsSet mine_positions;
        Position spawn_position;
        bool crashed;
    };

    typedef boost::array<Hero, 4> Heroes;

    State(const PTree& root, const HashedPair<Tiles>& background_tiles);

    void
    update(const PTree& root);

    void
    update(const Direction& direction);

    void
    status(std::ostream& os) const;

    int
    get_winner() const; // return -1 if no winner

    /// Return the rank of each player
    boost::array<int, 4>
    get_ranks() const;

    Tile
    get_tile_from_background(const Position& position) const;

    Tile
    get_tile_from_background_border_check(const Position& position) const;

    Heroes heroes;

    int next_hero_index;

private:

    Tile
    process_background_tile(const Tile& tile, const Position& position) const;

    Tiles
    get_tiles_full() const;

    void
    chain_respawn(const int& killed_hero_index, const int& killer_hero_index);

    friend
    Hash
    hash_value(const State& state);

    friend
    bool
    operator==(const State& state_aa, const State& state_bb);

    const HashedPair<Tiles> hashed_background_tiles;

};

std::ostream&
operator<<(std::ostream& os, const State& state);

Hash
hash_value(const State::Hero& hero);

bool
operator==(const State::Hero& hero_aa, const State::Hero& hero_bb);

