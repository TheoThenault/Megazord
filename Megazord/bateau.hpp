#pragma once

#include "../hlt/game.hpp"
#include "fsm.hpp"

/*
	Classe spécifique pour le comportement des bateaux
*/

class Bateau {
public:
	Bateau(hlt::Game* game);
	~Bateau();
	std::shared_ptr <hlt::Game> game;
	std::shared_ptr <hlt::Ship> ship;
	std::vector <hlt::Command> command_queue;
	FSM* m_state_machine;

	void decide(std::vector <hlt::Command>* command_queue, std::shared_ptr <hlt::Ship> ship);
	void collectHalites();
	void moveToHalites();
	void moveToStorage(hlt::Position dropoff);
	void moveToEnemies(hlt::Position enemies);
private:
	void setupStateMachine();
};