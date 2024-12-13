#include "bateau.hpp"
#include "game.hpp"
#include "dropoff.hpp"

#include "../hlt/log.hpp"



bateau::bateau(hlt::Game* game)
{
	bateau::game = game;
	hlt::log::log("BONDOUR bateau");
};

bateau::~bateau()
{
	hlt::log::log("AUREVOIE bateau");
};

char bateau::collect_halites()
{
	return 'o';
};

char bateau::move_to_halites()
{

};

char bateau::move_to_dropoff()
{
	std::unordered_map < hlt::EntityId, std::shared_ptr < hlt::Dropoff >> *dropoffs = &bateau::game->me->dropoffs;
	/*for (auto& iterator : *dropoffs)
	{	
		std::shared_ptr < hlt::Dropoff> dropoff = iterator.second;

	}*/
};

char bateau::move_to_enemies()
{

};