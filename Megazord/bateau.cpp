#include "bateau.hpp"
#include "../hlt/game.hpp"
#include "../hlt/dropoff.hpp"
#include "fsm.hpp"
#include "../hlt/game.hpp"
#include "../hlt/constants.hpp"
#include "../hlt/log.hpp"
#include "utils.hpp"

Bateau::Bateau(hlt::Game* _game, std::shared_ptr<hlt::Player> _player, Joueur* _joueur, hlt::EntityId _id)
{
	m_game		= _game;
	m_player	= _player;
	m_ship_id	= _id;
	m_joueur	= _joueur;

	setupStateMachine();
};

Bateau::~Bateau()
{
	delete m_state_machine;
};

float transRamasserHaliteToMoveToHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->m_game->game_map->at(bateau->ship)->halite < MINE_HALITE_THRESHOLD && 
		bateau->ship->halite < SHIP_FULL
		) {
		LOG(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToHalite 1");
		return 1.0f;
	}
	LOG(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToHalite 0.3");
	return 0.2f;
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
		closest_storage_distance * 1.0f, 1.0f, 0.5f
	);

	float fuzzy_logic = cargaison * distance_coeff;

	bateau->m_target_storage = closest_storage;

	// TODO : Envoyer directement la logique floue ?
	//if (fuzzy_logic > 0.7f)

	LOG(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToStorage " + std::to_string(fuzzy_logic)
		+ "(" + std::to_string(cargaison) + "/" + std::to_string(distance_coeff) + ")");
	return fuzzy_logic;

	//if (cargaison > 0.7f)
	//{
	//	hlt::log::log(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToStorage 1");
	//	return 1;
	//}
	LOG(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToStorage 0");
	//return 0;
}

float transMoveToHaliteToRamasserHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->m_game->game_map->at(bateau->ship)->halite >= MINE_HALITE_THRESHOLD)
	{
		LOG(std::to_string(bateau->ship->id) + " transMoveToHaliteToRamasserHalite 1");
		return 1;
	}
	else {
		LOG(std::to_string(bateau->ship->id) + " transMoveToHaliteToRamasserHalite 0");
		return 0;
	}
}

float transMoveToStorageToMoveToHalites(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->ship->halite == 0) {
		LOG(std::to_string(bateau->ship->id) + " transMoveToStorageToMoveToHalites 1");
		return 1;
	}
	else {
		LOG(std::to_string(bateau->ship->id) + " transMoveToStorageToMoveToHalites 0");
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
		LOG(std::to_string(bateau->ship->id) + " transAttackEnemie 1");
		return 1.0f;
	}
	LOG(std::to_string(bateau->ship->id) + " transAttackEnemie 0");
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

	hlt::Position closest_enemie = { 0, 0 };
	int closest_enemie_distance = 1000;
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
			}
		}
	}

	bateau->m_target_enemie = closest_enemie;


	if (diffNombreBateau(bateau->m_game, bateau->m_player, &(bateau->ship->position)) < 0) {
		LOG(std::to_string(bateau->ship->id) + " transFlee 1");
		return 1;
	}
	LOG(std::to_string(bateau->ship->id) + " transFlee 0");
	return 0;
}

float returnHome(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	int nbEnemie = nombreEnemies(bateau->m_game, bateau->m_player, &(bateau->ship->position));

	if (nbEnemie == 0)
	{
		LOG(std::to_string(bateau->ship->id) + " RETURNHOME 1");
		return 1;
	}
	LOG(std::to_string(bateau->ship->id) + " RETURNHOME 0");
	return 0;
}

float transKeepRamasser(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->ship->halite >= SHIP_FULL)
	{
		return 0;
	}

	hlt::Halite halite_under_ship = bateau->m_game->game_map->at(bateau->ship->position)->halite;

	float v = (halite_under_ship - KEEP_HARVESTING_THRESHOLD) / ((MAX_NATURAL_HALITE - KEEP_HARVESTING_THRESHOLD) * 1.0f);
	LOG(std::to_string(bateau->ship->id) + " keepRamasser " + std::to_string(v) + " (" + std::to_string(halite_under_ship) + ")");
	return v * 2.5f; // on a très envie de ramasser des halites
}

float transKeepMovingToHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	hlt::Position best_halite_position = bateau->ship->position;
	hlt::Halite best_halite = 0;

	for (int lookup_y = bateau->ship->position.y - SHIP_VIEW_DISTANCE; lookup_y <= bateau->ship->position.y + SHIP_VIEW_DISTANCE; lookup_y++)
	{
		int width = SHIP_VIEW_DISTANCE - ((lookup_y - bateau->ship->position.y) < 0 ? -(lookup_y - bateau->ship->position.y) : (lookup_y - bateau->ship->position.y));
		for (int lookup_x = bateau->ship->position.x - width; lookup_x <= bateau->ship->position.x + width; lookup_x++)
		{
			if (bateau->m_game->game_map->at({ lookup_x, lookup_y })->halite >= best_halite)
			{
				if (
					bateau->m_game->game_map->at({ lookup_x, lookup_y })->halite == best_halite &&
					bateau->m_game->game_map->calculate_distance(bateau->ship->position, best_halite_position) <
					bateau->m_game->game_map->calculate_distance(bateau->ship->position, { lookup_x, lookup_y })
					)
				{
					continue;
				}
				best_halite = bateau->m_game->game_map->at({ lookup_x, lookup_y })->halite;
				best_halite_position = { lookup_x, lookup_y };
			}
		}
	}

	int distance = bateau->m_game->game_map->calculate_distance(bateau->ship->position, best_halite_position);
	if (distance == 0)
		return 0.0f;


	float v = best_halite / 1000.0f;
	LOG(std::to_string(bateau->ship->id) + " transKeepMovingToHalite " + std::to_string(v));
	return v;
	LOG(std::to_string(bateau->ship->id) + " transKeepMovingToHalite 0");
	return 0.0f;
}

float transitionBoatToDropoff(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->m_joueur->boatAboutToTransform == bateau->m_ship_id)
		return 10000.0f;
	return -1.0f;
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

void wrpTransformDropoff(void* _data)
{
	Bateau* bateau = (Bateau*)_data;
	bateau->transformDropoff();
}

void Bateau::setupStateMachine()
{
	m_state_ramaser_halite  = new FSM_STATE(wrpCollectHalite);
	m_state_move_to_halite  = new FSM_STATE(wrpMoveToHalites);
	m_state_move_to_storage = new FSM_STATE(wrpMoveToTarget);
	m_state_move_to_enemie  = new FSM_STATE(wrpMoveToTarget);
	m_state_create_dropoff = new FSM_STATE(wrpTransformDropoff);

	// transition globale

	FSM_TRANSITION* trans_create_dropoff = new FSM_TRANSITION(transitionBoatToDropoff, m_state_create_dropoff);

	// Transitions state_ramaser_halite

	FSM_TRANSITION* trans_ramaser_halite_to_move_to_halite = new FSM_TRANSITION(transRamasserHaliteToMoveToHalite, m_state_move_to_halite);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_storage = new FSM_TRANSITION(transRamasserHaliteToMoveToStorage, m_state_move_to_storage);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_storage_flee = new FSM_TRANSITION(transFlee, m_state_move_to_storage);
	FSM_TRANSITION* trans_ramaser_halite_to_move_to_enemie = new FSM_TRANSITION(transAttackEnemie, m_state_move_to_enemie);
	FSM_TRANSITION* trans_keep_ramasser = new FSM_TRANSITION(transKeepRamasser, m_state_ramaser_halite);

	m_state_ramaser_halite->InitTransitions(6,
		trans_keep_ramasser,
		trans_ramaser_halite_to_move_to_halite,
		trans_ramaser_halite_to_move_to_storage,
		trans_ramaser_halite_to_move_to_storage_flee,
		trans_ramaser_halite_to_move_to_enemie,
		trans_create_dropoff
	);

	// Transitions state_move_to_halite
	FSM_TRANSITION* trans_move_to_halite_to_ramasser_halites = new FSM_TRANSITION(transMoveToHaliteToRamasserHalite, m_state_ramaser_halite);
	FSM_TRANSITION* trans_move_to_halite_to_move_to_enemie = new FSM_TRANSITION(transAttackEnemie, m_state_move_to_enemie);
	FSM_TRANSITION* trans_keep_moving_to_halite = new FSM_TRANSITION(transKeepMovingToHalite, m_state_move_to_halite);

	m_state_move_to_halite->InitTransitions(4,
		trans_move_to_halite_to_ramasser_halites,
		trans_move_to_halite_to_move_to_enemie,
		trans_keep_moving_to_halite,
		trans_create_dropoff
	);

	// Transitions state_move_to_storage
	FSM_TRANSITION* trans_move_to_dropoff_to_move_to_halites = new FSM_TRANSITION(transMoveToStorageToMoveToHalites, m_state_move_to_halite);

	m_state_move_to_storage->InitTransitions(2,
		trans_move_to_dropoff_to_move_to_halites,
		trans_create_dropoff
	);

	// Transitions state_move_to_enemie
	
	//FSM_TRANSITION* trans_move_to_enemie_to_ramasser_halite = new FSM_TRANSITION(transFlee, m_state_ramaser_halite);
	FSM_TRANSITION* trans_move_to_enemie_to_ramasser_halite = new FSM_TRANSITION(transFlee, m_state_move_to_storage);
	FSM_TRANSITION* trans_move_to_enemie_to_move_to_storage = new FSM_TRANSITION(returnHome, m_state_move_to_storage);

	m_state_move_to_enemie->InitTransitions(3,
		trans_move_to_enemie_to_ramasser_halite,
		trans_move_to_enemie_to_move_to_storage,
		trans_create_dropoff
	);

	m_state_machine = new FSM(4,
		m_state_ramaser_halite,
		m_state_move_to_halite,
		m_state_move_to_storage,
		m_state_move_to_enemie
	);
};

void Bateau::decide(std::vector <hlt::Command>* _command_queue, std::shared_ptr<hlt::Ship> _ship)
{
	if (_ship == nullptr)
		return;

	ship = _ship;

	command_queue = _command_queue;

	LOG(std::to_string(ship->id) + " " + lastState);


	//LOG(std::to_string(ship->id) + " Evaluate");
	m_current_state = m_state_machine->Evaluate(this);
	//LOG(std::to_string(ship->id) + " Behave");
	m_state_machine->Behave(this);
	//LOG(std::to_string(ship->id) + " End");

	//LOG(std::to_string(ship->id) + "[" + std::to_string(ship->position.x) + "," + std::to_string(ship->position.y) + "]");

};

void Bateau::collectHalites()
{
	LOG(std::to_string(ship->id) + " COLLECT HALITES");
	lastState = "COLLECT HALITES";
	
	m_game->game_map->at(ship->position)->mark_unsafe(ship);
	command_queue->push_back(ship->stay_still());
};

void Bateau::moveToHalites()
{
	LOG(std::to_string(ship->id) + " MOVE TO HALITES");
	lastState = "MOVE TO HALITES";
	
	hlt::Position best_halite_position = ship->position;
	hlt::Halite best_halite = 0;

	for (int lookup_y = ship->position.y - SHIP_VIEW_DISTANCE; lookup_y <= ship->position.y + SHIP_VIEW_DISTANCE; lookup_y++)
	{
		int width = SHIP_VIEW_DISTANCE - ((lookup_y - ship->position.y) < 0 ? -(lookup_y - ship->position.y) : (lookup_y - ship->position.y));
		for (int lookup_x = ship->position.x - width; lookup_x <= ship->position.x + width; lookup_x++)
		{
			if (m_game->game_map->at({ lookup_x, lookup_y })->halite >= best_halite)
			{
				if (
					m_game->game_map->at({ lookup_x, lookup_y })->halite == best_halite &&
					m_game->game_map->calculate_distance(ship->position, best_halite_position) <
					m_game->game_map->calculate_distance(ship->position, { lookup_x, lookup_y })
					)
				{	
					continue;
				}
				best_halite = m_game->game_map->at({ lookup_x, lookup_y })->halite;
				best_halite_position = { lookup_x, lookup_y };
			}
		}
	}

	LOG(std::to_string(ship->id) + " Best halite position" + posToStr(best_halite_position));

	hlt::Direction dir = m_game->game_map->naive_navigate(ship, best_halite_position);

	if (dir == hlt::Direction::STILL) {
		dir = directionAvailable(m_game, m_player, &(ship->position));

		m_game->game_map->at(ship->position.directional_offset(dir))->mark_unsafe(ship);
	}

	LOG(std::to_string(ship->id) + " DIRECTION " + dirToStr(dir));

	command_queue->push_back(ship->move(dir));
};

void Bateau::moveToTarget()
{
	if (m_current_state == m_state_move_to_enemie)
	{
		LOG(std::to_string(ship->id) + " MOVE TO ENEMIE");
		lastState = "MOVE TO ENEMIES";

		hlt::Direction dir = m_game->game_map->naive_navigate(ship, m_target_enemie);

		if (dir == hlt::Direction::STILL) {

			if (m_game->game_map->calculate_distance(ship->position, m_target_enemie) == 1)
			{
				std::shared_ptr<hlt::Ship> ship_at = m_game->game_map->at(m_target_enemie)->ship;
				if (ship_at != nullptr)
				{
					if (ship_at->owner != m_player->id)
					{
						m_game->game_map->at(m_target_enemie)->mark_unsafe(ship);

						if (m_target_enemie.x < ship->position.x)
							command_queue->push_back(ship->move(hlt::Direction::WEST));
						if (m_target_enemie.x > ship->position.x)
							command_queue->push_back(ship->move(hlt::Direction::EAST));
						if (m_target_enemie.y < ship->position.y)
							command_queue->push_back(ship->move(hlt::Direction::NORTH));
						if (m_target_enemie.y > ship->position.y)
							command_queue->push_back(ship->move(hlt::Direction::SOUTH));
						return;
					}
				}
			}
		}
		command_queue->push_back(ship->move(dir));
	}
	if (m_current_state == m_state_move_to_storage)
	{
		LOG(std::to_string(ship->id) + " MOVE TO STORAGE");
		lastState = "MOVE TO STORAGE";

		hlt::Direction dir = m_game->game_map->naive_navigate(ship, m_target_storage);

		if (dir == hlt::Direction::STILL)
			m_game->game_map->at(ship->position)->mark_unsafe(ship);

		command_queue->push_back(ship->move(dir));
	}
	
};

void Bateau::transformDropoff()
{
	command_queue->push_back(ship->make_dropoff());
	hlt::log::log("Create dropoff!");
}

int shipIndex(std::vector<Bateau*>* _bateaux, hlt::EntityId _id)
{
	for (unsigned int i = 0; i < _bateaux->size(); i++)
	{
		Bateau* bateau = (*_bateaux)[i];
		if (bateau->m_ship_id == _id)
			return i;
	}
	return -1;
}
