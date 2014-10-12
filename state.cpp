#include "state.h"

#include <vector>
#include <boost/functional/hash.hpp>

State::State(const PTree& root, const HashedPair<Tiles>& hashed_background_tiles) :
    next_hero_index(root.get<int>("game.turn") % 4),
    hashed_background_tiles(hashed_background_tiles)
{
    // init heroes
    const OwnedMines owned_mines = get_owned_mines(root.get_child("game.board"));

    int kk = 0;
    const PTree& child_heroes = root.get_child("game.heroes");
    for (PTree::const_iterator ti=child_heroes.begin(), tie=child_heroes.end(); ti!=tie; ti++)
    {
#if !defined(NDEBUG)
        const int id = ti->second.get<int>("id");
        assert( kk+1 == id );
#endif

        heroes[kk] = Hero(ti->second, owned_mines[kk]);

        assert( kk < 4 );
        kk++;
    }
}

void
State::update(const PTree& root)
{
    // update heroes
    const OwnedMines owned_mines = get_owned_mines(root.get_child("game.board"));

    int kk = 0;
    const PTree& child_heroes = root.get_child("game.heroes");
    for (PTree::const_iterator ti=child_heroes.begin(), tie=child_heroes.end(); ti!=tie; ti++)
    {
#if !defined(NDEBUG)
        const int id = ti->second.get<int>("id");
        assert( kk+1 == id );
#endif
        heroes[kk].update(ti->second, owned_mines[kk]);

        assert( kk < 4 );
        kk++;
    }

    // update next_hero_index
    next_hero_index = root.get<int>("game.turn") % 4;
}

void
State::chain_respawn(const int& killed_hero_index, const int& killer_hero_index)
{
    assert( killer_hero_index != killed_hero_index ); // no suicide

    Hero& killed_hero = heroes[killed_hero_index];

    int crushed_hero_index = -1;
    {
        const Tile& respawn_tile = get_tile_from_background(killed_hero.spawn_position);
        if (respawn_tile == HERO1) crushed_hero_index = 0;
        if (respawn_tile == HERO2) crushed_hero_index = 1;
        if (respawn_tile == HERO3) crushed_hero_index = 2;
        if (respawn_tile == HERO4) crushed_hero_index = 3;
    }

    killed_hero.position = killed_hero.spawn_position;
    killed_hero.life = 100;
    if (killer_hero_index >= 0) // steal mines
    {
        Hero& killer_hero = heroes[killer_hero_index];
        for (PositionsSet::const_iterator mi=killed_hero.mine_positions.begin(), mie=killed_hero.mine_positions.end(); mi!=mie; mi++)
            killer_hero.mine_positions.insert(*mi);
    }
    killed_hero.mine_positions.clear();

    if (crushed_hero_index < 0) return;
    if (killed_hero_index == crushed_hero_index) return; // dead on self spawning point

    assert( killed_hero_index != crushed_hero_index );

    chain_respawn(crushed_hero_index, killed_hero_index);
}


void
State::update(const Direction& direction)
{
    static const Tile hero_index_to_mines[4] = {
        MINE1,
        MINE2,
        MINE3,
        MINE4
    };

    static const int tile_to_hero_indexes[13] = {
        -1, // UNKNOWN
        -1, // EMPTY
        -1, // WOOD
        0,  // HERO1
        1,  // HERO2
        2,  // HERO3
        3,  // HERO4
        -1, // TAVERN
        -1, // MINE
        0,  // MINE1
        1,  // MINE2
        2,  // MINE3
        3,  // MINE4
    };


    assert( next_hero_index < 4 );
    const size_t hero_index = next_hero_index;
    Hero& hero = heroes[hero_index];

    /*{
        const Tiles& tiles_full = get_tiles_full();

        Tiles tiles_simple(hashed_background_tiles.value);
        std::fill(tiles_simple.origin(), tiles_simple.origin()+tiles_simple.num_elements(), UNKNOWN);
        const size_t* shape = tiles_simple.shape();
        for (size_t ii=0; ii<shape[0]; ii++)
            for (size_t jj=0; jj<shape[1]; jj++)
            {
                const Position position(ii,jj);
                tiles_simple[ii][jj] = get_tile_from_background(position);
            }

        assert( tiles_simple == tiles_full );
    }*/


    // move hero and resolve local interaction
    if (direction != STAY)
    {
        Position target_position = hero.position;
        target_position.with_direction(direction);
        const Tile& target_tile = get_tile_from_background_border_check(target_position);

        switch (target_tile)
        {
        case UNKNOWN:
        case WOOD:
            break;
        case EMPTY:
            hero.position = target_position;
            break;
        case TAVERN:
            if (hero.gold < 2) break;
            hero.gold -= 2;
            hero.life += 50;
            if (hero.life > 100) hero.life = 100;
            break;
        case HERO1:
        case HERO2:
        case HERO3:
        case HERO4:
            break;
        case MINE:
        case MINE1:
        case MINE2:
        case MINE3:
        case MINE4:
            if (target_tile == hero_index_to_mines[hero_index]) break;
            hero.life -= 20;
            if (hero.life <= 0) break;
            hero.mine_positions.insert(target_position);
            const int spoiled_hero_index = tile_to_hero_indexes[static_cast<int>(target_tile)];
            if (spoiled_hero_index < 0) break;
            Hero& spoiled_hero = heroes[spoiled_hero_index];
            assert( spoiled_hero.mine_positions.find(target_position) != spoiled_hero.mine_positions.end() );
            spoiled_hero.mine_positions.erase(target_position);
            break;
        }

    }

    // respawn if dead
    if (hero.life <= 0) chain_respawn(hero_index, -1);

    // resolve hero fights
    for (size_t kk=0; kk<heroes.size(); kk++)
    {
        if (kk == hero_index) continue;

        Hero& target_hero = heroes[kk];

        if (!target_hero.position.next_to(hero.position)) continue;

        assert( !(target_hero == hero) );
        target_hero.life -= 20;
        if (target_hero.life > 0) continue;

        chain_respawn(kk, hero_index);
    }

    // thirst
    if (hero.life > 1) hero.life--;

    // mining
    hero.gold += hero.mine_positions.size();

    // tick next_hero_index
    next_hero_index++;
    next_hero_index %= 4;
}

void
State::status(std::ostream& os) const
{
    os << std::hex;
    os << "hash " << hash_value(*this) << std::endl;
    os << std::dec;

    const int& winner_id = get_winner();
    const int colors[] = {31,34,32,33};
    for (int kk=0; kk<static_cast<int>(heroes.size()); kk++)
    {
        if (kk == winner_id) os << "*";
        else os << " ";
        if (kk == next_hero_index) os << ">";
        else os << " ";

        const State::Hero& hero = heroes[kk];

        os << "@" << (kk+1) << " " << "\033[" << colors[kk] << "m";
        os << hero.life << "hp ";
        os << hero.gold << "g ";
        os << hero.mine_positions.size() << "m";
        os << "\033[0m" << std::endl;
    }

    os << get_tiles_full();
}

std::ostream&
operator<<(std::ostream& os, const State& state)
{
    return os << "<State @" << (state.next_hero_index+1) << " " << std::hex << hash_value(state) << std::dec << ">";
}

Hash
hash_value(const State& state)
{
    Hash seed = 5465763;
    boost::hash_range(seed, state.heroes.begin(), state.heroes.end());
    boost::hash_combine(seed, state.hashed_background_tiles.hash);
    boost::hash_combine(seed, state.next_hero_index);
    return seed;
}

bool
operator==(const State& state_aa, const State& state_bb)
{
    if (state_aa.next_hero_index !=  state_bb.next_hero_index) return false;
    if (state_aa.hashed_background_tiles != state_bb.hashed_background_tiles) return false;

    return std::equal(state_aa.heroes.begin(), state_aa.heroes.end(), state_bb.heroes.begin());
}

typedef std::pair<int, int> GoldIdPair;
typedef std::vector<GoldIdPair> GoldIdPairs;

static
bool
sort_gold_id_pair(const GoldIdPair& pair_aa, const GoldIdPair& pair_bb)
{
    return pair_aa.first > pair_bb.first;
}

int
State::get_winner() const
{
    GoldIdPairs gold_ids;
    for (int kk=0; kk<4; kk++)
        gold_ids.push_back(std::make_pair(heroes[kk].gold, kk));

    std::sort(gold_ids.begin(), gold_ids.end(), sort_gold_id_pair);

    GoldIdPairs::const_iterator gi = gold_ids.begin();
    if (gi == gold_ids.end()) return -1;

    const int winner_id = gi->second;
    const int winner_gold = gi->first;
    gi++;
    if (gi == gold_ids.end()) return winner_id;

    const int second_gold = gi->first;
    if (second_gold == winner_gold) return -1;

    return winner_id;
}

boost::array<int, 4>
State::get_ranks() const {
    boost::array<int, 4> res;
    GoldIdPairs gold_ids;
    for (int kk=0; kk<4; kk++)
        gold_ids.push_back(std::make_pair(heroes[kk].gold, kk));

    std::sort(gold_ids.begin(), gold_ids.end(), sort_gold_id_pair);
    for (int kk=0; kk<4; kk++) {
        res[gold_ids[kk].second] = kk;
    }
    return res;
}

Tile
State::process_background_tile(const Tile& tile, const Position& position) const
{
    static const Tile hero_tiles[4] = {HERO1, HERO2, HERO3, HERO4};
    static const Tile hero_mine_tiles[4] = {MINE1, MINE2, MINE3, MINE4};

    switch (tile)
    {
    case UNKNOWN:
    case WOOD:
    case TAVERN:
        return tile;
    case HERO1:
    case HERO2:
    case HERO3:
    case HERO4:
    case MINE1:
    case MINE2:
    case MINE3:
    case MINE4:
        assert(false);
        return UNKNOWN;
    case MINE:
        for (int kk=0; kk<4; kk++)
        {
            const Hero& hero = heroes[kk];
            if (hero.mine_positions.find(position) != hero.mine_positions.end())
                return hero_mine_tiles[kk];
        }
        return tile;
    case EMPTY:
        for (int kk=0; kk<4; kk++)
            if (heroes[kk].position == position)
                return hero_tiles[kk];
        return tile;
    }

    assert(false);
    return UNKNOWN;
}

Tile
State::get_tile_from_background(const Position& position) const
{
    return process_background_tile(get_tile(hashed_background_tiles.value, position), position);
}

Tile
State::get_tile_from_background_border_check(const Position& position) const
{
    return process_background_tile(get_tile_border_check(hashed_background_tiles.value, position), position);
}

Tiles
State::get_tiles_full() const
{
    Tiles tiles(hashed_background_tiles.value);

    static const Tile hero_tiles[4] = {HERO1, HERO2, HERO3, HERO4};
    static const Tile hero_mine_tiles[4] = {MINE1, MINE2, MINE3, MINE4};

    for (int kk=0; kk<4; kk++)
    {
        const Hero& hero = heroes[kk];
        get_tile(tiles, hero.position) = hero_tiles[kk];
        for (PositionsSet::const_iterator mi=hero.mine_positions.begin(), mie=hero.mine_positions.end(); mi!=mie; mi++)
        {
            Tile& tile = get_tile(tiles, *mi);
            assert( tile == MINE );
            tile = hero_mine_tiles[kk];
        }
    }

    return tiles;
}

State::Hero::Hero() :
    position(Position()),
    life(-1),
    gold(-1),
    mine_positions(),
    spawn_position(Position()),
    crashed(true)
{
}

State::Hero::Hero(const PTree& root, const PositionsSet& mine_positions) :
    position(get_position(root.get_child("pos"))),
    life(root.get<int>("life")),
    gold(root.get<int>("gold")),
    mine_positions(mine_positions),
    spawn_position(get_position(root.get_child("spawnPos"))),
    crashed(root.get<bool>("crashed"))
{
    assert( root.get<size_t>("mineCount") == mine_positions.size() );
}

void
State::Hero::update(const PTree& root, const PositionsSet& mine_positions)
{
    this->position = get_position(root.get_child("pos"));
    this->life = root.get<int>("life");
    this->gold = root.get<int>("gold");
    this->crashed = root.get<bool>("crashed");
    this->mine_positions = mine_positions;

    assert( root.get<size_t>("mineCount") == mine_positions.size() );
}

Hash
hash_value(const State::Hero& hero)
{
    Hash seed = 4546139;
    boost::hash_combine(seed, hero.position);
    boost::hash_combine(seed, hero.life);
    boost::hash_combine(seed, hero.gold);
    boost::hash_combine(seed, hero.crashed);
    boost::hash_combine(seed, hero.spawn_position);
    boost::hash_range(seed, hero.mine_positions.begin(), hero.mine_positions.end());
    return seed;
}

bool
operator==(const State::Hero& hero_aa, const State::Hero& hero_bb)
{
    if (hero_aa.position !=  hero_bb.position) return false;
    if (hero_aa.life != hero_bb.life) return false;
    if (hero_aa.gold != hero_bb.gold) return false;
    if (hero_aa.crashed != hero_bb.crashed) return false;
    if (hero_aa.spawn_position != hero_bb.spawn_position) return false;

    if (hero_aa.mine_positions.size() != hero_bb.mine_positions.size()) return false;
    return std::equal(hero_aa.mine_positions.begin(), hero_aa.mine_positions.end(), hero_bb.mine_positions.begin());
}


