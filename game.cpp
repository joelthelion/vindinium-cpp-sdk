#include "game.h"

#include <boost/regex.hpp>

Game::Game(const PTree& root) :
    background_tiles(get_background_tiles(root.get_child("game.board"))),
    hashed_background_tiles(make_hashed_pair(background_tiles)),
    turn_max(root.get<int>("game.maxTurns")),
    turn(root.get<int>("game.turn")),
    state(root, hashed_background_tiles)
{
    assert( state == state );
    assert( hash_value(state) == hash_value(state) );

    int kk = 0;
    const PTree& child_heroes = root.get_child("game.heroes");
    for (PTree::const_iterator ti=child_heroes.begin(), tie=child_heroes.end(); ti!=tie; ti++)
    {
#if !defined(NDEBUG)
        const int id = ti->second.get<int>("id");
        assert( kk+1 == id );
#endif

        const_cast<HeroInfo&>(hero_infos[kk]) = HeroInfo(ti->second);

        assert( kk < 4 );
        kk++;
    }

}

void
Game::status(std::ostream& os) const
{
    os << "turn " << turn << "/" << turn_max;
    if (is_finished()) os << " finished";
    os << std::endl;

    const int winner_id = state.get_winner();
    const int colors[] = {31,34,32,33};
    for (int kk=0; kk<static_cast<int>(hero_infos.size()); kk++)
    {
        const HeroInfo& hero_info = hero_infos[kk];

        if (kk == winner_id) os << "* ";
        else os << "  ";

        os << "@" << (kk+1) << " ";
        os << "\033[" << colors[kk] << "m" << hero_info.name << "\033[0m";
        if (hero_info.is_real_bot()) os << "(" << hero_info.elo << ")";
        if (hero_info.crashed) {
            std::cout << " (crashed)";
        }
        os << std::endl;
    }

    state.status(os);
}

bool
Game::is_finished() const
{
    return turn >= turn_max;
}

void
Game::update(const PTree& root)
{
    // update turn
    assert( root.get<int>("game.maxTurns") == turn_max );
    turn = root.get<int>("game.turn");
    assert( state.next_hero_index == turn % 4 );
}

void
Game::update(const Direction&)
{
    turn ++;
    assert( state.next_hero_index == turn % 4 );
}

Game::HeroInfo::HeroInfo() :
    name("unknown"),
    user_id(""),
    elo(-1)
{
}

Game::HeroInfo::HeroInfo(const PTree& root) :
    name(root.get<std::string>("name")),
    user_id(root.get("userId", "")),
    elo(root.get("elo", -1)),
    crashed(root.get("crashed",false))
{
}

bool
Game::HeroInfo::is_real_bot() const
{
    return elo >= 0;
}
