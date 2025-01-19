#include "bateau.hpp"
#include "../hlt/game.hpp"
#include "../hlt/dropoff.hpp"
#include "fsm.hpp"
#include "../hlt/game.hpp"
#include "../hlt/constants.hpp"
#include "../hlt/log.hpp"
#include "utils.hpp"



Bateau::Bateau(hlt::Game* _game)
{
	ship = nullptr;

	m_game = _game;

	setupStateMachine();

	hlt::log::log("BONDOUR bateau");
};

Bateau::~Bateau()
{
	delete m_state_machine;
	hlt::log::log("AUREVOIE bateau");
};

float transRamasserHaliteToMoveToHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if(bateau->ship != nullptr)

	if (bateau->m_game->game_map->at(bateau->ship)->halite < 350) {
		hlt::log::log(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToHalite 1");
		return 1;
	}
	else {
		hlt::log::log(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToHalite 0");
		return 0;
	}
}

float transRamasserHaliteToMoveToStorage(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	int halite_in_ship = bateau->ship->halite;
	float cargaison = halite_in_ship / ((float)MAX_HALITE_IN_SHIP);

	int closest_storage_distance = bateau->m_game->game_map->calculate_distance(
		bateau->ship->position, bateau->m_player->shipyard->position
	);
	hlt::Position closest_storage = bateau->m_player->shipyard->position;

	for (auto& dropoff_pair : bateau->m_player->dropoffs)
	{
		std::shared_ptr<hlt::Dropoff> dropoff = dropoff_pair.second;
		int distance = bateau->m_game->game_map->calculate_distance(
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
		closest_storage_distance, 1.0f, 0.0f
	);

	float fuzzy_logic = cargaison * distance_coeff;

	bateau->m_target_storage = closest_storage;

	// TODO : Envoyer directement la logique floue ?
	//if (fuzzy_logic > 0.7f)
	if (cargaison > 0.7f)
	{
		hlt::log::log(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToStorage 1");
		return 1;
	}
	hlt::log::log(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToStorage 0");
	return 0;
}

float transMoveToHaliteToRamasserHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->m_game->game_map->at(bateau->ship)->halite >= MINE_HALITE_THRESHOLD)
	{
		hlt::log::log(std::to_string(bateau->ship->id) + " transMoveToHaliteToRamasserHalite 1");
		return 1;
	}
	else {
		hlt::log::log(std::to_string(bateau->ship->id) + " transMoveToHaliteToRamasserHalite 0");
		return 0;
	}
}

float transMoveToStorageToMoveToHalites(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->ship->halite == 0) {
		hlt::log::log(std::to_string(bateau->ship->id) + " transMoveToStorageToMoveToHalites 1");
		return 1;
	}
	else {
		hlt::log::log(std::to_string(bateau->ship->id) + " transMoveToStorageToMoveToHalites 0");
		return 0;
	}
}

float transAttackEnemie(void* _data)
{

	Bateau* bateau = (Bateau*)_data;

	int halite_in_ship = bateau->ship->halite;
	float cargaison = halite_in_ship / ((float)MAX_HALITE_IN_SHIP);

	//decide which enemie TODO: choose the best one 

	hlt::Position closest_enemie = { 0, 0 };
	int closest_enemie_distance = 1000;
	int closest_enemie_halite = 0;
	for (auto& player : bateau->m_game->players)
	{
		if (player == bateau->m_player)
			continue;

		for (auto& ship_pair : player->ships)
		{
			int distance = bateau->m_game->game_map->calculate_distance(
				bateau->ship->position, ship_pair.second->position
			);

			if (distance < closest_enemie_distance && distance < ENEMIE_SEARCH_RADIUS)
			{
				closest_enemie_distance = distance;
				closest_enemie = ship_pair.second->position;
				closest_enemie_halite = ship_pair.second->halite;
			}
		}
	}

	bateau->m_target_enemie = closest_enemie;

	float cargaison_enemie = closest_enemie_halite / ((float)MAX_HALITE_IN_SHIP);

	int threatened = diffNombreBateau(bateau->m_game, bateau->m_player, &(bateau->ship->position)) < DIFF_NOMBRE_BATEAU_THREAT_THRESHOLD;

	if (((1-threatened) * cargaison_enemie * 1/cargaison) > 0.5f) {
		hlt::log::log(std::to_string(bateau->ship->id) + " transAttackEnemie 1");
		return 1.0f;
	}
	hlt::log::log(std::to_string(bateau->ship->id) + " transAttackEnemie 0");
	return 0;
}

float transFlee(void* _data)
{
	//TODO c'est un peu bête il faudrait un vrai état de fuite pour pouvoir en sortir si on est hors de danger
	// remarque : on ne distingue pas les 3 enemies
	Bateau* bateau = (Bateau*)_data;

	// TODO : Nearest dropoff en une fonction

	int closest_storage_distance = bateau->m_game->game_map->calculate_distance(
		bateau->ship->position, bateau->m_player->shipyard->position
	);
	hlt::Position closest_storage = bateau->m_player->shipyard->position;

	for (auto& dropoff_pair : bateau->m_player->dropoffs)
	{
		std::shared_ptr<hlt::Dropoff> dropoff = dropoff_pair.second;
		int distance = bateau->m_game->game_map->calculate_distance(
			bateau->ship->position, dropoff->position
		);

		if (distance < closest_storage_distance)
		{
			closest_storage_distance = distance;
			closest_storage = dropoff->position;
		}
	}
	
	bateau->m_target_storage = closest_storage;

	if (diffNombreBateau(bateau->m_game, bateau->m_player, &(bateau->ship->position)) < 0) {
		hlt::log::log(std::to_string(bateau->ship->id) + " transFlee 1");
		return 1;
	}
	else {
		hlt::log::log(std::to_string(bateau->ship->id) + " transFlee 0");
		return 0;
	}
}

float returnHome(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	int nbEnemie = nombreEnemies(bateau->m_game, bateau->m_player, &(bateau->ship->position));

	if (nbEnemie == 0)
	{
		return 1;
	}
	else{
		return 0;
	}
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

void wrpMoveToTarget(void* _data)
{
	Bateau* bateau = (Bateau*)_data;
	bateau->moveToTarget();
}


void Bateau::setupStateMachine()
{
	m_state_ramaser_halite  = new FSM_STATE(wrpCollectHalite);
	m_state_move_to_halite  = new FSM_STATE(wrpMoveToHalites);
	m_state_move_to_storage = new FSM_STATE(wrpMoveToTarget);
	m_state_move_to_enemie  = new FSM_STATE(wrpMoveToTarget);

	// Transitions state_ramaser_halite

	FSM_TRANSITION* trans_ramaser_halite_to_move_to_halite = new FSM_TRANSITION(transRamasserHaliteToMoveToHalite, m_state_move_to_halite);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_storage = new FSM_TRANSITION(transRamasserHaliteToMoveToStorage, m_state_move_to_storage);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_storage_flee = new FSM_TRANSITION(transFlee, m_state_move_to_storage);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_enemie = new FSM_TRANSITION(transAttackEnemie, m_state_move_to_enemie);

	m_state_ramaser_halite->InitTransitions(4,
		trans_ramaser_halite_to_move_to_halite,
		trans_ramaser_halite_to_move_to_storage,
		trans_ramaser_halite_to_move_to_storage_flee,
		trans_ramaser_halite_to_move_to_enemie
	);

	// Transitions state_move_to_halite
	FSM_TRANSITION* trans_move_to_halite_to_ramasser_halites = new FSM_TRANSITION(transMoveToHaliteToRamasserHalite, m_state_ramaser_halite);
	FSM_TRANSITION* trans_move_to_halite_to_move_to_enemie = new FSM_TRANSITION(transAttackEnemie, m_state_move_to_enemie);

	m_state_move_to_halite->InitTransitions(2,
		trans_move_to_halite_to_ramasser_halites,
		trans_ramaser_halite_to_move_to_enemie
		//trans_move_to_halite_to_move_to_enemie
	);

	// Transitions state_move_to_storage
	FSM_TRANSITION* trans_move_to_dropoff_to_move_to_halites = new FSM_TRANSITION(transMoveToStorageToMoveToHalites, m_state_move_to_halite);

	m_state_move_to_storage->InitTransitions(1,
		trans_move_to_dropoff_to_move_to_halites
	);

	// Transitions state_move_to_enemie
	
	//FSM_TRANSITION* trans_move_to_enemie_to_ramasser_halite = new FSM_TRANSITION(transFlee, m_state_ramaser_halite);
	FSM_TRANSITION* trans_move_to_enemie_to_ramasser_halite = new FSM_TRANSITION(transFlee, m_state_move_to_storage);
	FSM_TRANSITION* trans_move_to_enemie_to_move_to_storage = new FSM_TRANSITION(returnHome, m_state_move_to_storage);

	m_state_move_to_enemie->InitTransitions(2,
		trans_move_to_enemie_to_ramasser_halite,
		trans_move_to_enemie_to_move_to_storage
	);

	m_state_machine = new FSM(4,
		m_state_ramaser_halite,
		m_state_move_to_halite,
		m_state_move_to_storage,
		m_state_move_to_enemie
	);
};

void Bateau::decide(std::vector <hlt::Command>* _command_queue, std::shared_ptr <hlt::Ship> _ship, std::shared_ptr<hlt::Player> _player)
{
	ship = _ship;
	command_queue = _command_queue;
	m_player = _player;

	hlt::log::log(std::to_string(ship->id) + " " + debug_lastState);

	//hlt::log::log(std::to_string(ship->id) + " Evaluate");
	m_state_machine->Evaluate(this);
	//hlt::log::log(std::to_string(ship->id) + " Behave");
	m_current_state = m_state_machine->Behave(this);
	//hlt::log::log(std::to_string(ship->id) + " End");

};

//TODO: mark unsafe the cells the ship will be on next turn to avoid collisions

void Bateau::collectHalites()
{
	hlt::log::log(std::to_string(ship->id) + " COLLECT HALITES");
	debug_lastState = "COLLECT HALITES";
	
	m_game->game_map->at(ship->position)->mark_unsafe(ship);
	command_queue->push_back(ship->stay_still());
};

void Bateau::moveToHalites()
{
	hlt::log::log(std::to_string(ship->id) + " MOVE TO HALITES");
	debug_lastState = "MOVE TO HALITES";
	
	hlt::Direction best_direction = hlt::Direction::STILL;
	hlt::Halite best_direction_halite = 0;

	for (unsigned int index_dir = 0; index_dir < 4; index_dir++)
	{
		hlt::Halite direction_halite = m_game->game_map->at(ship->position.directional_offset(hlt::ALL_CARDINALS[index_dir]))->halite;
		bool unsafe = m_game->game_map->at(ship->position.directional_offset(hlt::ALL_CARDINALS[index_dir]))->is_occupied();

		if (direction_halite > best_direction_halite && !unsafe)
		{
			best_direction_halite = direction_halite;
			best_direction = hlt::ALL_CARDINALS[index_dir];
		}
	}

	m_game->game_map->at(ship->position.directional_offset(best_direction))->mark_unsafe(ship);
	command_queue->push_back(ship->move(best_direction));
};

void Bateau::moveToTarget()
{
	if (m_current_state == m_state_move_to_enemie)
	{
		hlt::log::log(std::to_string(ship->id) + " MOVE TO ENEMIE");
		debug_lastState = "MOVE TO ENEMIE";

		hlt::Direction dir = m_game->game_map->naive_navigate(ship, m_target_enemie);

		if(dir == hlt::Direction::STILL)
			m_game->game_map->at(ship->position)->mark_unsafe(ship);

		command_queue->push_back(ship->move(dir));
	}
	if (m_current_state == m_state_move_to_storage)
	{
		hlt::log::log(std::to_string(ship->id) + " MOVE TO STORAGE");
		debug_lastState = "MOVE TO STORAGE";

		hlt::Direction dir = m_game->game_map->naive_navigate(ship, m_target_storage);

		if (dir == hlt::Direction::STILL)
			m_game->game_map->at(ship->position)->mark_unsafe(ship);

		command_queue->push_back(ship->move(dir));
	}
	
};
