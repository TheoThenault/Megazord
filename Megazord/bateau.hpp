#pragma once

#include "../hlt/game.hpp"
#include "fsm.hpp"

/*
	Classe spécifique pour le comportement des bateaux
*/

class Bateau {
public:
	Bateau(hlt::Game*, std::shared_ptr<hlt::Player>, std::shared_ptr <hlt::Ship>);
	~Bateau();
	
	hlt::Game* m_game;
	std::shared_ptr<hlt::Player> m_player;
	std::shared_ptr <hlt::Ship> m_ship;
	FSM* m_state_machine;

	hlt::Position m_target_storage;
	hlt::Position m_target_enemie;

	std::vector <hlt::Command>* command_queue;

	void decide(std::vector <hlt::Command>*);
	void collectHalites();
	void moveToHalites();
	void moveToTarget();

private:
	void setupStateMachine();

	std::string debug_lastState = "";

	FSM_STATE*	m_current_state			= nullptr;
	FSM_STATE*	m_state_ramaser_halite	= nullptr;
	FSM_STATE*	m_state_move_to_halite	= nullptr;
	FSM_STATE*	m_state_move_to_storage	= nullptr;
	FSM_STATE*	m_state_move_to_enemie	= nullptr;
};

int shipIndex(std::vector<Bateau*>*, std::shared_ptr<hlt::Ship>);
