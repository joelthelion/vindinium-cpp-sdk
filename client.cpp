#include "game.h"
#include BOTINCLUDE
#include "state.h"
#include "utils.h"
#include "network.h"
#include "options.h"
#include "tiles.h"

#include <signal.h>
#include <boost/regex.hpp>
#include <cassert>
#include <fstream>

#if defined(OPENMP_FOUND)
#include <omp.h>
#endif

#define TURN_DURATION .8

Game
play_game(const Options& options, Rng& rng)
{
    static const boost::regex re_url("^(?:http://)?([^/]+)(/.*)$");

    HTTPConnection connection(options.server_name);
    connection.proxy = options.proxy;
    const PTree& initial_json = connection.get_initial_state_json(options);
    //std::cout << initial_json;

    boost::match_results<std::string::const_iterator> what;
    if (!regex_search(initial_json.get<std::string>("playUrl"), what, re_url))
        throw std::runtime_error("can't parse play url");
    const std::string play_server_name(what[1].first, what[1].second);
    const std::string play_end_point(what[2].first, what[2].second);

    const std::string& view_url = initial_json.get<std::string>("viewUrl");
    std::cout << "view game at " << view_url << std::endl;
    double start_time = get_double_time();

    if (options.collect_map) { // collect maps
        const Tiles tiles = get_tiles(initial_json.get_child("game.board"));
        const HashedPair<Tiles> hashed_tiles(tiles);
        std::stringstream ss;
        ss << "map_" << std::hex << hashed_tiles.hash << std::dec << ".txt";
        std::cout << "saving " << ss.str() << std::endl;
        std::ofstream handle(ss.str().c_str());
        handle << hashed_tiles.value;
        handle.close();
    }

    Game game(initial_json);
    Bot bot(options, game, rng);

    while (!game.is_finished())
    {
        OmpFlag continue_flag(true);

        std::cout << std::endl;
        std::cout << "======================================== " << clock_it(get_double_time() - start_time) << std::endl;

        game.status(std::cout);

        std::cout << "++++++++++++++++++++++++++++++++++++++++ " << clock_it(get_double_time() - start_time) << std::endl;

        const Direction direction = bot.get_move(game);
        std::cout << "bot direction " << direction << std::endl;

        bot.advance_game(game, direction);

        std::cout << "---------------------------------------- " << clock_it(get_double_time() - start_time) << std::endl;

        std::cout << "view game at " << view_url << std::endl;

        PTree new_json;
        double request_start_time;
        double request_end_time;
#if defined(OPENMP_FOUND)
        #pragma omp parallel sections default(shared) shared(new_json, request_end_time, request_start_time, continue_flag, play_end_point, play_server_name, start_time, bot, game)
        {

            #pragma omp section
#endif
            {
                request_start_time = get_double_time();
                new_json = connection.get_new_state_json(play_end_point, direction);
                request_end_time = get_double_time();

                continue_flag.reset();
            }
        }
        game.state.update(new_json);
        game.update(new_json);

        std::cout << "request took " << clock_it(request_end_time-request_start_time) << std::endl;

        std::cout << "======================================== " << clock_it(get_double_time() - start_time) << std::endl;
        start_time = get_double_time();
    }

    assert( game.is_finished() );

    return game;
}

// Allow exiting infinite game loops without losing a game
static bool sigint_already_caught=false;

void sigint_handler(int signum) {
    if (sigint_already_caught) {
        std::cerr << "\nSIGINT caught for the second time, exiting immediately!" << std::endl;
        exit(1);
    } else {
        std::cerr << "\nSIGINT caught for the first time, ignoring..." << std::endl;
        sigint_already_caught = true;
    }
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigint_handler);
#if !defined(NDEBUG)
    std::cout << "\t> Running in DEBUG mode" << std::endl;
#endif

#if defined(OPENMP_FOUND)
    omp_set_nested(true);
    std::cout << "\t> Running using OPENMP " << std::endl;
    std::cout << "\t\t> " << omp_get_max_threads() << " threads max" << std::endl;
    std::cout << "\t\t> " << omp_get_wtick()*1e9 << "ns tick" << std::endl;
    assert( omp_get_nested() );
#endif

//     test_random();
    Rng rng;
    rng.seed(rand());

    Options options = parse_options(argc, argv);

    typedef std::map<std::string, int> Wins;
    Wins wins;

    for (int kk=0; kk<options.number_of_games; kk++)
    {
        std::cout << std::endl << std::endl;
        std::cout << "****************************************" << std::endl;
        std::cout << "game " << kk << "/" << options.number_of_games << std::endl;

        const Game& game = play_game(options, rng);

        const int winner = game.state.get_winner();
        if (winner < 0) wins["draw"]++;
        else {
            std::string winner_name = "bot";
            if (game.hero_infos[winner].is_real_bot())
                winner_name = game.hero_infos[winner].name;
            wins[winner_name]++;
        }

        std::cout << std::endl;
        std::cout << "after " << options.number_of_games << " games" << std::endl;
        for (Wins::const_iterator wi=wins.begin(), wie=wins.end(); wi!=wie; wi++)
        {
            if (wi->first == "draw")
            {
                std::cout << "  " << wi->second << " draw" << std::endl;
                continue;
            }
            std::cout << "  " << wi->second << " victory for " << wi->first << std::endl;
        }

        if (sigint_already_caught) break;
    }

    return 0;
}

