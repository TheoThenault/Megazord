#include "utils.hpp"
#include "../hlt/game.hpp"
#include "../hlt/ship.hpp"
#include "../hlt/game_map.hpp"

#include <fstream>
#include <iostream>

/*
	Transforme une valeur `val` présente entre `smin` et `smax` et la retourne
	présente entre `omin` et `omax`
*/
float map(float smin, float smax, float val, float omin, float omax)
{
	float percent = (val - smin) / (smax - smin);
	return (omax - omin) * percent + omin;
}


int random(int min, int max) {
	return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

int diffNombreBateau(hlt::Game* _game, std::shared_ptr<hlt::Player> _player, hlt::Position* _center)
{
	int diff = 0;
	
	for (auto& ship_pair : _player->ships)
	{
		if (_game->game_map->calculate_distance(*_center, ship_pair.second->position) < DIFF_NOMBRE_BATEAU_RADIUS)
		{
			diff++;
		}
	}

	diff -= nombreEnemies(_game, _player, _center);

	return diff;
}

int nombreEnemies(hlt::Game* _game, std::shared_ptr<hlt::Player> _player, hlt::Position* _center)
{
	int diff = 0;

	for (std::shared_ptr<hlt::Player> player : _game->players)
	{
		if (player == _player)
			continue;

		for (auto& ship_pair : player->ships)
		{
			if (_game->game_map->calculate_distance(*_center, ship_pair.second->position) < DIFF_NOMBRE_BATEAU_RADIUS)
			{
				diff++;
			}
		}
	}

	return diff;
}

bool isOccupied(hlt::Game* _game, std::shared_ptr<hlt::Player> _player, hlt::Position _pos)
{
	if (_game->game_map->at(_pos)->is_occupied())
	{
		for (const auto& ship_pair : _player->ships)
		{
			if (ship_pair.second == _game->game_map->at(_pos)->ship)
			{
				return true;
			}
		}
	}
	return false;
}

hlt::Direction directionAvailable(hlt::Game* _game, std::shared_ptr<hlt::Player> _player, hlt::Position* _pos)
{
	if (!isOccupied(_game, _player, _pos->directional_offset(hlt::Direction::NORTH)))
		return hlt::Direction::NORTH;
	if (!isOccupied(_game, _player, _pos->directional_offset(hlt::Direction::SOUTH)))
		return hlt::Direction::SOUTH;
	if (!isOccupied(_game, _player, _pos->directional_offset(hlt::Direction::WEST)))
		return hlt::Direction::WEST;
	if (!isOccupied(_game, _player, _pos->directional_offset(hlt::Direction::EAST)))
		return hlt::Direction::EAST;
	return hlt::Direction::STILL;
}

std::string posToStr(hlt::Position _pos)
{
	return "{" + std::to_string(_pos.x) + "," + std::to_string(_pos.y) + "}";
}

std::string dirToStr(hlt::Direction _dir)
{
	switch (_dir)
	{
	case hlt::Direction::NORTH:
		return "NORTH";
		break;
	case hlt::Direction::EAST:
		return "EAST";
		break;
	case hlt::Direction::SOUTH:
		return "SOUTH";
		break;
	case hlt::Direction::WEST:
		return "WEST";
		break;
	case hlt::Direction::STILL:
		return "STILL";
		break;
	default:
		break;
	}
}
