#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

#include "Megazord/hfsm.hpp"

#include <random>
#include <ctime>
#include "Megazord/rules.hpp"
#include "Megazord/joueur.hpp"

using namespace std;
using namespace hlt;

#ifdef _DEBUG
# define LOG(X) log::log(X);
#else
# define LOG(X)
#endif // DEBUG

int main(int argc, char* argv[]) {
    unsigned int rng_seed;
    if (argc > 1) {
        rng_seed = static_cast<unsigned int>(stoul(argv[1]));
    } else {
        rng_seed = static_cast<unsigned int>(time(nullptr));
    }
    mt19937 rng(rng_seed);

    Game game;
    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    game.ready("Kayissa&Theo");

    LOG("Bondour");

    HFSM state_machine;

    Joueur joueur(&game);

    for (;;) {
        game.update_frame();
        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;

        vector<Command> command_queue;

        joueur.setCommandQueue(&command_queue);
        joueur.think(me);

        for (const auto& ship_iterator : me->ships) {
            shared_ptr<Ship> ship = ship_iterator.second;
            if (game_map->at(ship)->halite < constants::MAX_HALITE / 10 || ship->is_full()) {
                Direction random_direction = ALL_CARDINALS[rng() % 4];
                command_queue.push_back(ship->move(random_direction));
            } else {
                command_queue.push_back(ship->stay_still());
            }
        }

        //if (
        //    game.turn_number <= 200 &&
        //    me->halite >= constants::SHIP_COST &&
        //    !game_map->at(me->shipyard)->is_occupied())
        //{
        //    command_queue.push_back(me->shipyard->spawn());
        //}

        if (!game.end_turn(command_queue)) {
            break;
        }
    }

    return 0;
}
