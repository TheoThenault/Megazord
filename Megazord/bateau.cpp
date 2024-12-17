#include "bateau.hpp"
#include "../hlt/game.hpp"
#include "../hlt/dropoff.hpp"
#include "hfsm.hpp"
#include "../hlt/game.hpp"
#include "../hlt/constants.hpp"

#include "../hlt/log.hpp"



Bateau::Bateau(hlt::Game* game)
{
	game = game;
	hlt::log::log("BONDOUR bateau");
};

Bateau::~Bateau()
{
	hlt::log::log("AUREVOIE bateau");
};

void Bateau::decide(std::vector <hlt::Command>* command_queue, HFSM* state_machine, std::shared_ptr <hlt::Ship> ship)
{
	ship = ship;
	command_queue = command_queue;
};

void Bateau::collectHalites()
{
	ship->stay_still();
};

void Bateau::moveToHalites()
{

};

void Bateau::moveToStorage(hlt::Position dropoff)
{
	ship->move(game->game_map->naive_navigate(ship, dropoff));

};

void Bateau::moveToEnemies(hlt::Position enemies)
{

};

/* code pour décider si on y va à un dropoff
	std::unordered_map < hlt::EntityId, std::shared_ptr < hlt::Dropoff >> *dropoffs = &bateau::game->me->dropoffs;
	for (auto& iterator : *dropoffs)
	{
		std::shared_ptr < hlt::Dropoff> dropoff = iterator.second;
		int distance = game->game_map->calculate_distance(ship->position, dropoff->position);
		// TODO: check for the nearest before sending the ship
		if ((-distance * ship->halite) < 1000) //Value to adapt
		{
			// return move_to_dropoff(dropoff->position);
		}
	}*/