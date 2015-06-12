#include "options.h"

#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/errors.hpp>
namespace po = boost::program_options;

Options parse_options(int argc, char* argv[])
{
    Options options;
    options.server_name = "";
    options.map_name = "";

    po::options_description po_options("client [options]");
    po_options.add_options()
        ("help,h", "display this message")
        ("number-of-turns,n", po::value<int>(&options.number_of_turns)->default_value(60), "number of turns in training mode")
        ("number-of-games,g", po::value<int>(&options.number_of_games)->default_value(1), "number of games played")
        ("training,t", po::value<bool>(&options.training_mode)->default_value(true), "training or arena")
        ("secret-key,k", po::value<std::string>(&options.secret_key)->default_value(""), "secret api key")
        ("server,s", po::value<std::string>(&options.server_name)->default_value("vindinium.org"), "server name")
        ("map,m", po::value<std::string>(&options.map_name)->default_value(""), "map name")
        ("proxy", po::value<std::string>(&options.proxy)->default_value(""), "SOCKS proxy to use (eg. localhost:4444)")
        ("collect-map", po::value<bool>(&options.collect_map)->default_value(false), "save game map");
    po::positional_options_description positional;

    try
    {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(po_options).positional(positional).run(), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << po_options;
            std::exit(0);
        }

        if (options.secret_key.size() != 8) throw po::invalid_option_value("secret_key.size != 8");
        if (options.number_of_turns < 0) throw po::invalid_option_value("number_of_turns < 0");
        if (options.number_of_games < 0) throw po::invalid_option_value("number_of_games < 0");
    }
    catch (std::exception& ex)
    {
        std::cerr << "Error occurred when parsing options: " << ex.what() << std::endl;
        std::cerr << po_options;
        std::exit(1);
    }

    return options;
}
