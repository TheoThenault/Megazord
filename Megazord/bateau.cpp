#include "bateau.hpp"
#include "../hlt/game.hpp"
#include "../hlt/dropoff.hpp"
#include "fsm.hpp"
#include "../hlt/game.hpp"
#include "../hlt/constants.hpp"

#include "../hlt/log.hpp"



Bateau::Bateau(hlt::Game* game)
{
	game = game;

	setupStateMachine();

	hlt::log::log("BONDOUR bateau");
};

Bateau::~Bateau()
{
	hlt::log::log("AUREVOIE bateau");
};

float transRamasserHaliteToMoveToHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;
	if (bateau->game->game_map->at(bateau->ship)->halite < 350)
		return 1;
	else
		return 0;
}

void wrpCollectHalite(void* _data)
{
	Bateau* bateau = (Bateau*) _data;
	bateau->collectHalites();
}

void Bateau::setupStateMachine()
{
	FSM_STATE* state_ramaser_halite  = new FSM_STATE(wrpCollectHalite);
	FSM_STATE* state_move_to_halite  = nullptr; // TODO
	FSM_STATE* state_move_to_storage = nullptr; // TODO
	FSM_STATE* state_move_to_enemie  = nullptr; // TODO

	FSM_TRANSITION* trans_ramaser_halite_to_move_to_halite = new FSM_TRANSITION(transRamasserHaliteToMoveToHalite, state_move_to_halite);

	state_ramaser_halite->InitTransitions(4,
		trans_ramaser_halite_to_move_to_halite,
		// TODO
	);

	m_state_machine = new FSM(4,
		state_ramaser_halite,
		state_move_to_halite,
		state_move_to_storage,
		state_move_to_enemie
	);
};

void Bateau::decide(std::vector <hlt::Command>* command_queue, std::shared_ptr <hlt::Ship> ship)
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