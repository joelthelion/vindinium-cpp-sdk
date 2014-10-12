#pragma once

#include <cstdlib>

typedef std::size_t Hash;

template <typename Value>
struct HashedPair
{
    HashedPair(const Value& value) :
        hash(hash_value(value)),
        value(value)
    {
    }

    const Hash hash;
    const Value& value;
};

template <typename Value>
HashedPair<Value>
make_hashed_pair(const Value& value)
{
    return HashedPair<Value>(value);
}

template <typename Value>
bool
operator==(const HashedPair<Value>& hashed_value_aa, const HashedPair<Value>& hashed_value_bb)
{
    return hashed_value_aa.hash == hashed_value_aa.hash;
}

template <typename Value>
bool
operator!=(const HashedPair<Value>& hashed_value_aa, const HashedPair<Value>& hashed_value_bb)
{
    return hashed_value_aa.hash != hashed_value_aa.hash;
}

