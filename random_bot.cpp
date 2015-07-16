#include "random_bot.h"

Bot::Bot(const Options& opt, const Game& game, Rng& rng) :
    rng(rng)
{
}

Direction
Bot::get_move(const Game& game) const
{
    typedef UniformRng<uint8_t> UniformRngUInt8;
    UniformRngUInt8 uniform(rng, 5);
    return static_cast<Direction>(uniform());
}

void
Bot::advance_game(Game& game, const Direction& direction)
{
}

