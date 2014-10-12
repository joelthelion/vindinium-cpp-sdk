#pragma once

#include "utils.h"
#include "options.h"

#include <iostream>
#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>

#include "position.h"
#include "tiles.h"

typedef void CURL;

typedef boost::property_tree::ptree PTree;
typedef std::map<std::string, std::string> Params;

class HTTPConnection {
public:
    HTTPConnection(const std::string& server);
    ~HTTPConnection();
    std::string
    get_http(const std::string& end_point, const Params& params);

    PTree
    get_json(const std::string& end_point, const Params& params);
    PTree
    get_initial_state_json(const Options& options);
    PTree
    get_new_state_json(const std::string& end_point, const Direction& direction);
    std::string proxy;
protected:
    std::string server;
    CURL* curl;
};

PTree
get_initial_state_json(const Options& options);

PTree
get_new_state_json(const std::string& server, const std::string& end_point, const Direction& direction);

Position
get_position(const PTree& root);

Tiles
get_tiles(const PTree& root);

Tiles
get_background_tiles(const PTree& root);

OwnedMines
get_owned_mines(const PTree& root);

std::ostream&
operator<<(std::ostream& os, const PTree& root);

