#pragma once

#include <string>

struct Options
{
    int number_of_turns;
    int number_of_games;
    bool training_mode;
    std::string secret_key;
    std::string server_name;
    std::string map_name;
    std::string proxy;
    double uct_constant;
    int max_mc_depth;
    bool collect_map;
};

Options
parse_options(int argc, char* argv[]);

