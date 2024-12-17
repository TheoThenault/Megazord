#pragma once

#include "game.hpp"

/*
	Classe spécifique pour le comportement des bateaux
*/

class bateau {
public:
	bateau(hlt::Game* game);
	~bateau();
	std::shared_ptr <hlt::Game> game;
	std::shared_ptr <hlt::Ship> ship;
	std::vector <hlt::Command> command_queue;

	void decide(std::vector <hlt::Command>* command_queue, std::shared_ptr <hlt::Ship> ship);
	void collect_halites();
	void move_to_halites();
	void move_to_dropoff(hlt::Position dropoff);
	void move_to_enemies(hlt::Position enemies);
private:

};