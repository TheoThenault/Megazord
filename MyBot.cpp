#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

#include "Megazord/wheeloffortune.hpp"

#include <random>
#include <ctime>
#include "Megazord/rules.hpp"
#include "Megazord/joueur.hpp"
#include "Megazord/bateau.hpp"
#include "Megazord/fsm.hpp"
#include "Megazord/utils.hpp"
#include <string>

using namespace std;
using namespace hlt;

//#ifdef _DEBUG
//# define LOG(X) log::log(X);
//#else
//# define LOG(X)
//#endif // DEBUG

int main(int argc, char* argv[]) {
    unsigned int rng_seed;
    if (argc > 1) {
        rng_seed = static_cast<unsigned int>(stoul(argv[1]));
    } else {
        rng_seed = static_cast<unsigned int>(time(nullptr));
    }
    mt19937 rng(rng_seed);
    srand(rng_seed);

    Game game;
    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.

    Joueur joueur(&game);

    std::vector<Bateau*> bateaux;

    game.ready("Kayyissa&Theo");

    for (;;) {
        game.update_frame();
        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;

        vector<Command> command_queue;

        LOG("HALITE " + to_string(me->halite));

        for (const auto& ship_iterator : me->ships) {
            shared_ptr<Ship> ship = ship_iterator.second;
            /*if (game_map->at(ship)->halite < constants::MAX_HALITE / 10 || ship->is_full()) {
                Direction random_direction = ALL_CARDINALS[rng() % 4];
                command_queue.push_back(ship->move(random_direction));
            } else {
                command_queue.push_back(ship->stay_still());
            }*/

            int ship_index = shipIndex(&bateaux, ship->id);
            if (ship_index == -1)
            {
                LOG("Nouveau bateau pour id : " + std::to_string(ship->id));
                Bateau* bateau = new Bateau(&game, me, &joueur, ship->id);
                bateau->decide(&command_queue, ship);
                bateaux.push_back(bateau);
            }
            else {
                //LOG("Bateau existant pour id : " + std::to_string(ship->id));
                bateaux[ship_index]->decide(&command_queue, ship);
            }

        }

        joueur.setCommandQueue(&command_queue);
        joueur.think(me);

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

    for (Bateau* b : bateaux)
        delete b;
    bateaux.clear();

    return 0;
}
