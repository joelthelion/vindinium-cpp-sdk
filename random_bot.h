#pragma once

#include "game.h"

struct Bot
{
    Bot(const Game& game, Rng& rng);

    void
    crunch_it_baby(const Game& game, const OmpFlag& continue_flag, const double& start_time, const double& duration);

    Direction
    get_move(const Game& game) const;

    void
    advance_game(Game& game, const Direction& direction);

private:

    Rng& rng;

};


