#include "bateau.hpp"
#include "../hlt/game.hpp"
#include "../hlt/dropoff.hpp"
#include "fsm.hpp"
#include "../hlt/game.hpp"
#include "../hlt/constants.hpp"
#include "../hlt/log.hpp"
#include "utils.hpp"


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

	m_current_state = m_state_machine->Evaluate(this);
	m_state_machine->Behave(this);
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
	LOG("Create dropoff!");
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
