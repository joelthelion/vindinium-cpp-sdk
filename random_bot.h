#pragma once

#include "game.h"

struct Bot
{
    Bot(const Options& opt, const Game& game, Rng& rng);

    Direction
    get_move(const Game& game) const;

    void
    advance_game(Game& game, const Direction& direction);

private:

    Rng& rng;

};


