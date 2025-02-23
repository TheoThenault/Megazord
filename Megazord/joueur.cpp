#include "joueur.hpp"
#include "../hlt/log.hpp"
#include "../hlt/ship.hpp"
#include "utils.hpp"

#define DESIRE_SPAWNBOAT "SpawnBoat"
#define DESIRE_CREATEDROPOFF "CreateDropoff"

static void ruleSpawnBoat(void* _joueur)
{
	Joueur* joueur = (Joueur*)_joueur;
	if (joueur->m_game->turn_number < MAX_TURN_NUMBER / 2)
	{
		if (joueur->m_player->halite > PLAYER_MIN_HALITE_THRESHOLD)
		{
			joueur->m_rule_engine->setDesire(DESIRE_SPAWNBOAT, 1);
		}
	}
	else if (joueur->m_game->turn_number < MAX_TURN_NUMBER * (3 / 4))
	{
		if (joueur->m_player->halite > COST_CREATE_DROPOFF + PLAYER_MIN_HALITE_THRESHOLD + COST_SPAWN_BOAT)
		{
			joueur->m_rule_engine->setDesire(DESIRE_SPAWNBOAT, 1);
		}
	}
}

static void ruleCreateDropoff(void* _joueur)
{
	Joueur* joueur = (Joueur*)_joueur;

	if (joueur->m_player->halite > COST_CREATE_DROPOFF && 
		joueur->m_game->turn_number < MAX_TURN_NUMBER * (3 / 4.0f))
	{
		for (auto& id_ship_pair : joueur->m_player->ships)
		{
			int halite = joueur->m_game->game_map->at(id_ship_pair.second)->halite;
			halite += id_ship_pair.second->halite;

			LOG("[" + std::to_string(halite) + "]");

			if (halite > SHIP_TRANSFORM_INTO_DROPOFF)
			{
				joueur->m_rule_engine->setDesire(DESIRE_CREATEDROPOFF, 1);
			}
		}

	}
}

static void ruleWatchoutWhenSpawning(void* _joueur)
{
	Joueur* joueur = (Joueur*)_joueur;

	if (joueur->m_rule_engine->getDesire(DESIRE_SPAWNBOAT) == 1)
	{
		for (auto& ship_pair : joueur->m_player->ships)
		{
			if (ship_pair.second->position == joueur->m_player->shipyard->position)
			{
				joueur->m_rule_engine->setDesire(DESIRE_SPAWNBOAT, 0);
			}
			if (joueur->m_game->game_map->at(joueur->m_player->shipyard->position)->is_occupied())
			{
				joueur->m_rule_engine->setDesire(DESIRE_SPAWNBOAT, 0);
			}
		}
	}
}

static void ruleCapSpawnBoat(void* _joueur)
{
	Joueur* joueur = (Joueur*)_joueur;

	if (joueur->m_player->ships.size() >= MAX_AMOUNT_OF_BOAT)
	{
		joueur->m_rule_engine->setDesire(DESIRE_SPAWNBOAT, 0);
	}
}

Joueur::Joueur(hlt::Game* _game) : m_game(_game)
{
	m_value = 0;

	m_expected_halite = 0;

	m_command_queue = nullptr;

	m_rule_engine = new Rules("Joueur", (void*)this);

	m_rule_engine->addRule("ruleSpawnBoat", ruleSpawnBoat);
	m_rule_engine->addRule("ruleCreateDropoff", ruleCreateDropoff);
	m_rule_engine->addRule("ruleWatchoutWhenSpawning", ruleWatchoutWhenSpawning);
	m_rule_engine->addRule("ruleCapSpawnBoat", ruleCapSpawnBoat);
}

Joueur::~Joueur()
{
	delete m_rule_engine;
}

void Joueur::setCommandQueue(std::vector<hlt::Command>* _queue)
{
	m_command_queue = _queue;
}

void Joueur::think(std::shared_ptr<hlt::Player> _player)
{
	m_player = _player;

	m_expected_halite = m_player->halite;

	if (m_boatAboutToTransform != -1)
	{
		m_expected_halite -= COST_CREATE_DROPOFF;
		m_boatAboutToTransform = -1;
	}

	std::string action = m_rule_engine->infer();

	if (action == DESIRE_SPAWNBOAT)
	{
		spawnBoat();
	}

	if (action == DESIRE_CREATEDROPOFF)
	{
		createDropoff();
	}
}

void Joueur::createDropoff()
{
	// TODO : Improve the choice of the best ship

	std::vector<hlt::Position> structure_pos;
	structure_pos.push_back(m_player->shipyard->position);
	for (const auto& dropoff_pair : m_player->dropoffs)
	{
		structure_pos.push_back(dropoff_pair.second->position);
	}

	std::vector<std::shared_ptr<hlt::Ship>> eligible_ships;
	int best_halite = 0;
	size_t best_index = -1;
	for (auto& id_ship_pair : m_player->ships)
	{
		int halite = m_game->game_map->at(id_ship_pair.second)->halite;
		halite += id_ship_pair.second->halite;

		if (halite > SHIP_TRANSFORM_INTO_DROPOFF)
		{
			bool far_enough = true;
			for (const hlt::Position& pos : structure_pos)
			{
				int distance = m_game->game_map->calculate_distance(
					id_ship_pair.second->position, pos
				);
				if (distance < DISTANCE_BETWEEN_STORAGE)
					far_enough = false;
			}

			if (far_enough)
			{
				eligible_ships.push_back(id_ship_pair.second);
				if (halite > best_halite)
				{
					best_halite = halite;
					best_index = eligible_ships.size() - 1;
				}
			}
		}
	}

	if (best_index != -1)
	{
		if (m_expected_halite - COST_CREATE_DROPOFF > PLAYER_MIN_HALITE_THRESHOLD)
		{
			m_boatAboutToTransform = eligible_ships[best_index]->id;
			LOG("ABOUT TO CREATE DROPOFF");
		}
	}
	

}

void Joueur::spawnBoat()
{
	if (m_expected_halite - COST_SPAWN_BOAT > PLAYER_MIN_HALITE_THRESHOLD)
	{
		m_command_queue->push_back(m_player->shipyard->spawn());
		m_expected_halite -= COST_SPAWN_BOAT;
		LOG("Spawn boat!");
	}
}