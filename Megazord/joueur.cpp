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
		if (joueur->m_player->halite > 1000)
		{
			joueur->m_rule_engine->setDesire(DESIRE_SPAWNBOAT, 1);
		}
	}
	else if (joueur->m_game->turn_number < MAX_TURN_NUMBER * (3 / 4))
	{
		if (joueur->m_player->halite > 5000)
		{
			joueur->m_rule_engine->setDesire(DESIRE_SPAWNBOAT, 1);
		}
	}
}

static void ruleCreateDropoff(void* _joueur)
{
	Joueur* joueur = (Joueur*)_joueur;
	if (joueur->m_player->halite > 4000)
	{
		for (auto& id_ship_pair : joueur->m_player->ships)
		{
			int halite = joueur->m_game->game_map->at(id_ship_pair.second)->halite;
			halite += id_ship_pair.second->halite;

			if (halite > 1500)
			{
				joueur->m_rule_engine->setDesire(DESIRE_CREATEDROPOFF, 1);
			}
		}

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
	std::vector<std::shared_ptr<hlt::Ship>> eligible_ships;
	for (auto& id_ship_pair : m_player->ships)
	{
		int halite = m_game->game_map->at(id_ship_pair.second)->halite;
		halite += id_ship_pair.second->halite;

		if (halite > 1500)
		{
			m_rule_engine->setDesire(DESIRE_CREATEDROPOFF, 1);
			eligible_ships.push_back(id_ship_pair.second);
		}
	}

	// TODO : Sort for best candidate

	for (auto& ship : eligible_ships)
	{
		if (m_expected_halite - COST_CREATE_DROPOFF > MIN_HALITE_THRESHOLD)
		{
			m_command_queue->push_back(ship->make_dropoff());
			m_expected_halite -= COST_CREATE_DROPOFF;
			hlt::log::log("Create dropoff!");
		}
	}
}

void Joueur::spawnBoat()
{
	if (m_expected_halite - COST_SPAWN_BOAT > MIN_HALITE_THRESHOLD)
	{
		m_command_queue->push_back(m_player->shipyard->spawn());
		m_expected_halite -= COST_SPAWN_BOAT;
		hlt::log::log("Spawn boat!");
	}
}