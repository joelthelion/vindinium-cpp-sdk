#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>

typedef boost::random::mt19937 Rng;

void
test_random();

template <typename Integer>
struct UniformRng
{
    typedef boost::random::uniform_smallint<Integer> Distribution;

    UniformRng(Rng& rng, const Integer& size) :
        rng(rng),
        distribution(0, size-1)
    {
    }

    Integer
    operator()()
    {
        return distribution(rng);
    }

    Rng& rng;
    const Distribution distribution;
};

template <typename Integer>
struct SizeRng
{
    typedef boost::random::uniform_smallint<Integer> Distribution;

    SizeRng(Rng& rng) :
        rng(rng)
    {
    }

    Integer
    operator()(const Integer& size_prime)
    {
        const Distribution distribution_prime(0, size_prime-1);
        return distribution_prime(rng);
    }

    Rng& rng;
};

/****************************************/

template <typename T>
inline
std::string
to_string(const T& x)
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}

/****************************************/

enum Direction
{
    STAY,
    NORTH,
    SOUTH,
    EAST,
    WEST
};

std::ostream&
operator<<(std::ostream& os, const Direction& direction);

/****************************************/

double
get_double_time();

struct clock_it
{
    clock_it(const double& delta);

    double delta;
};

std::ostream&
operator<<(std::ostream& os, const clock_it& clock_it);

/****************************************/

struct OmpFlag
{
    OmpFlag(const bool& initial_state);

    void
    set();

    void
    reset();

    bool
    test() const;

private:

    bool state;
};

