#include "bateau_transitions.hpp"
#include "bateau.hpp"
#include "joueur.hpp"
#include "utils.hpp"
#include "../hlt/game.hpp"
#include "../hlt/dropoff.hpp"
#include "../hlt/game.hpp"
#include "../hlt/constants.hpp"

hlt::Position findClosestStorage(Bateau* _bateau)
{
	int closest_storage_distance = _bateau->m_game->game_map->calculate_distance(
		_bateau->ship->position, _bateau->m_player->shipyard->position
	);
	hlt::Position closest_storage = _bateau->m_player->shipyard->position;

	for (auto& dropoff_pair : _bateau->m_player->dropoffs)
	{
		std::shared_ptr<hlt::Dropoff> dropoff = dropoff_pair.second;
		int distance = _bateau->m_game->game_map->calculate_distance(
			_bateau->ship->position, dropoff->position
		);

		if (distance < closest_storage_distance)
		{
			closest_storage_distance = distance;
			closest_storage = dropoff->position;
		}
	}
	return closest_storage;
}

hlt::Position findClosestEnemie(Bateau* _bateau)
{
	hlt::Position closest_enemie = { -1, -1 };
	int closest_enemie_distance = 1000;
	int closest_enemie_halite = 0;
	for (auto& player : _bateau->m_game->players)
	{
		if (player == _bateau->m_player)
			continue;

		for (auto& ship_pair : player->ships)
		{
			int distance = _bateau->m_game->game_map->calculate_distance(
				_bateau->ship->position, ship_pair.second->position
			);

			if (distance < closest_enemie_distance && distance < ENEMIE_SEARCH_RADIUS)
			{
				closest_enemie_distance = distance;
				closest_enemie = ship_pair.second->position;
				closest_enemie_halite = ship_pair.second->halite;
			}
		}
	}
	return closest_enemie;
}

float transRamasserHaliteToMoveToHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->m_game->game_map->at(bateau->ship)->halite < MINE_HALITE_THRESHOLD
		&& bateau->ship->halite < SHIP_FULL)
	{
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

	hlt::Position closest_storage = findClosestStorage(bateau);
	int closest_storage_distance = bateau->m_game->game_map->calculate_distance(
		bateau->ship->position, closest_storage
	);
	
	float distance_coeff = map(
		STORAGE_DISTANCE_CLOSE, STORAGE_DISTANCE_FAR,
		closest_storage_distance * 1.0f, 1.0f, 0.5f
	);

	float fuzzy_logic = cargaison * distance_coeff;

	bateau->m_target_storage = closest_storage;

	LOG(std::to_string(bateau->ship->id) + " transRamasserHaliteToMoveToStorage " + std::to_string(fuzzy_logic)
		+ "(" + std::to_string(cargaison) + "/" + std::to_string(distance_coeff) + ")");
	return fuzzy_logic;
}

float transMoveToHaliteToRamasserHalite(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->m_game->game_map->at(bateau->ship)->halite >= MINE_HALITE_THRESHOLD)
	{
		LOG(std::to_string(bateau->ship->id) + " transMoveToHaliteToRamasserHalite 1");
		return 1;
	}
	LOG(std::to_string(bateau->ship->id) + " transMoveToHaliteToRamasserHalite 0");
	return 0;
}

float transMoveToStorageToMoveToHalites(void* _data)
{
	Bateau* bateau = (Bateau*)_data;

	if (bateau->ship->halite == 0) {
		LOG(std::to_string(bateau->ship->id) + " transMoveToStorageToMoveToHalites 1");
		return 1;
	}
	LOG(std::to_string(bateau->ship->id) + " transMoveToStorageToMoveToHalites 0");
	return 0;
}

float transAttackEnemie(void* _data)
{

	Bateau* bateau = (Bateau*)_data;

	int halite_in_ship = bateau->ship->halite;
	float cargaison = halite_in_ship / ((float)MAX_HALITE_IN_SHIP);

	bateau->m_target_enemie = findClosestEnemie(bateau);

	std::shared_ptr<hlt::Ship> enemie_ship = bateau->m_game->game_map->at(bateau->m_target_enemie)->ship;

	int closest_enemie_halite = enemie_ship == nullptr ? 0 : enemie_ship->halite;

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
	// remarque : on ne distingue pas les 3 enemies
	Bateau* bateau = (Bateau*)_data;

	// TODO : Nearest dropoff en une fonction

	hlt::Position closest_storage = findClosestStorage(bateau);
	int closest_storage_distance = bateau->m_game->game_map->calculate_distance(
		bateau->ship->position, closest_storage
	);
	
	bateau->m_target_storage = closest_storage;

	bateau->m_target_enemie = findClosestEnemie(bateau);

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

	if (bateau->m_joueur->m_boatAboutToTransform == bateau->m_ship_id)
		return 10000.0f;
	return -1.0f;
}

