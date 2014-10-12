#pragma once

#include "hashed.h"
#include "state.h"

struct Game
{
    struct HeroInfo
    {
        HeroInfo();
        HeroInfo(const PTree& root);

        bool
        is_real_bot() const;

        std::string name;
        std::string user_id; // optional
        int elo; // optional
        bool crashed; // optional
    };

    typedef boost::array<HeroInfo, 4> HeroInfos;

    Game(const PTree& root);

    bool
    is_finished() const;

    void
    update(const Direction& direction);

    void
    update(const PTree& root);

    void
    status(std::ostream& os) const;

    const Tiles background_tiles;
    const HashedPair<Tiles> hashed_background_tiles;
    const HeroInfos hero_infos;

    const int turn_max;
    int turn;

    State state;

private:

    Game&
    operator=(const Game& game); // no assignement

};

