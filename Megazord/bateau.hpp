#pragma once

#include "game.hpp"

/*
	Classe sp�cifique pour le comportement des bateaux
*/

class bateau {
public:
	bateau(hlt::Game* game);
	~bateau();
	hlt::Game* game;

	char collect_halites();
	char move_to_halites();
	char move_to_dropoff();
	char move_to_enemies();
;private:

};