#include "utils.hpp"
#include "../hlt/game.hpp"
#include "../hlt/ship.hpp"
#include "../hlt/game_map.hpp"

/*
	Transforme une valeur `val` présente entre `smin` et `smax` et la retourne
	présente entre `omin` et `omax`
*/
float map(float smin, float smax, float val, float omin, float omax)
{
	float percent = (val - smin) / (smax - smin);
	return (omax - omin) * percent + omin;
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

	for (std::shared_ptr<hlt::Player> player : _game->players)
	{
		if (player == _player)
			continue;

		for (auto& ship_pair : player->ships)
		{
			if (_game->game_map->calculate_distance(*_center, ship_pair.second->position) < DIFF_NOMBRE_BATEAU_RADIUS)
			{
				diff--;
			}
		}
	}

}