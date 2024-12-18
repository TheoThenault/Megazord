#include "bateau.hpp"
#include "../hlt/game.hpp"
#include "../hlt/dropoff.hpp"
#include "fsm.hpp"
#include "../hlt/game.hpp"
#include "../hlt/constants.hpp"
#include "../hlt/log.hpp"
#include "utils.hpp"


Bateau::Bateau(hlt::Game* game, std::shared_ptr<hlt::Player> _player)
{
	game = game;
	m_player = _player;

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

float transRamasserHaliteToMoveToStorage(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	int halite_in_ship = bateau->ship->halite;
	float cargaison = halite_in_ship / ((float)MAX_HALITE_IN_SHIP);

	int closest_storage_distance = bateau->game->game_map->calculate_distance(
		bateau->ship->position, bateau->m_player->shipyard->position
	);
	hlt::Position closest_storage = bateau->m_player->shipyard->position;

	for (auto& dropoff_pair : bateau->m_player->dropoffs)
	{
		std::shared_ptr<hlt::Dropoff> dropoff = dropoff_pair.second;
		int distance = bateau->game->game_map->calculate_distance(
			bateau->ship->position, dropoff->position
		);

		if (distance < closest_storage_distance)
		{
			closest_storage_distance = distance;
			closest_storage = dropoff->position;
		}
	}
	
	float distance_coeff = map(
		STORAGE_DISTANCE_CLOSE, STORAGE_DISTANCE_FAR,
		closest_storage_distance, 0.0f, 1.0f
	);

	float fuzzy_logic = cargaison * distance_coeff;

	// TODO : Envoyer directement la logique floue ?
	if (fuzzy_logic > 0.7f)
	{
		return 1;
	}
	return 0;
}

float transMoveToHaliteToRamasserHalite(void* _data) {
	Bateau* bateau = (Bateau*)_data;
	if (bateau->game->game_map->at(bateau->ship)->halite >= 350)
		return 1;
	else
		return 0;
}

float transMoveToStorageToMoveToHalites(void* _data) {
	Bateau* bateau = (Bateau*)_data;
	if (bateau->game->game_map->at(bateau->ship)->halite == 0)
		return 1;
	else
		return 0;
}

float transRamasserHaliteToMoveToEnemie(void* _data) {
	Bateau* bateau = (Bateau*)_data;
	int halite_in_ship = bateau->ship->halite;
	float cargaison = halite_in_ship / ((float)MAX_HALITE_IN_SHIP);

	//decide which enemie TODO: choose the best one 

	bateau->game->players

	for (const auto& ship_iterator : me->ships) {
		int halite_in_enemie_ship = ();
		float cargaison_enemie = halite_in_enemie_ship / ((float)MAX_HALITE_IN_SHIP);

		if ((diffnbBateau * cargaison_enemie * 1/cargaison) > 0.5f) {

		}
	}
}

float transRamasserHaliteToMoveStorageFlee(void* _data) {
	//TODO c'est un peu bête il faudrait un vrai état de fuite pour pouvoir en sortir si on est hors de danger
	// remarque : on ne distingue pas les 3 enemies
	if (diffnbBateau > 0)
		return 1;
	else
		return 0;
}

void wrpCollectHalite(void* _data)
{
	Bateau* bateau = (Bateau*) _data;
	bateau->collectHalites();
}

void wrpMoveToHalites(void* _data)
{
	Bateau* bateau = (Bateau*)_data;
	bateau->moveToHalites();
}

void wrpMoveToStorage(void* _data)
{
	Bateau* bateau = (Bateau*)_data;
	bateau->moveToStorage(bateau->m_target);
}

void wrpMoveToEnemies(void* _data)
{
	Bateau* bateau = (Bateau*)_data;
	bateau->moveToStorage(bateau->m_target);
}

void Bateau::setupStateMachine()
{
	FSM_STATE* state_ramaser_halite  = new FSM_STATE(wrpCollectHalite);
	FSM_STATE* state_move_to_halite  = new FSM_STATE(wrpMoveToHalites);
	FSM_STATE* state_move_to_storage = new FSM_STATE(wrpMoveToStorage);
	FSM_STATE* state_move_to_enemie  = new FSM_STATE(wrpMoveToEnemies);

	// Transitions state_ramaser_halite

	FSM_TRANSITION* trans_ramaser_halite_to_move_to_halite = new FSM_TRANSITION(transRamasserHaliteToMoveToHalite, state_move_to_halite);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_storage = new FSM_TRANSITION(transRamasserHaliteToMoveToStorage, state_move_to_storage);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_storage_flee = new FSM_TRANSITION(transRamasserHaliteToMoveStorageFlee, state_move_to_storage);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_enemie = new FSM_TRANSITION(transRamasserHaliteToMoveToEnemie, state_move_to_enemie);

	state_ramaser_halite->InitTransitions(4,
		trans_ramaser_halite_to_move_to_halite,
		trans_ramaser_halite_to_move_to_storage,
		trans_ramaser_halite_to_move_to_storage_flee,
		trans_ramaser_halite_to_move_to_enemie
	);

	// Transitions state_move_to_halite
	FSM_TRANSITION* trans_move_to_halite_to_ramasser_halites = new FSM_TRANSITION(transMoveToHaliteToRamasserHalite, state_ramaser_halite);

	state_move_to_halite->InitTransitions(1,
		trans_move_to_halite_to_ramasser_halites);

	// Transitions state_move_to_storage
	FSM_TRANSITION* trans_move_to_dropoff_to_move_to_halites = new FSM_TRANSITION(transMoveToStorageToMoveToHalites, state_move_to_halite);

	state_move_to_halite->InitTransitions(1,
		trans_move_to_dropoff_to_move_to_halites);

	// Transitions state_move_to_enemie

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

//TODO: mark unsafe the cells the ship will be on next turn to avoid collisions

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