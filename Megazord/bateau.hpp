#pragma once

#include "../hlt/game.hpp"
#include "fsm.hpp"

/*
	Classe spécifique pour le comportement des bateaux
*/

class Bateau {
public:
	Bateau(hlt::Game* _game);
	~Bateau();
	hlt::Game* m_game;
	std::shared_ptr<hlt::Player> m_player;
	std::shared_ptr <hlt::Ship> ship;
	std::vector <hlt::Command>* command_queue;
	FSM* m_state_machine;
	hlt::Position m_target_storage;
	hlt::Position m_target_enemie;

	void decide(std::vector <hlt::Command>* command_queue, std::shared_ptr <hlt::Ship> ship, std::shared_ptr<hlt::Player> _player);
	void collectHalites();
	void moveToHalites();
	void moveToTarget();
private:
	void setupStateMachine();

	std::string debug_lastState = "";

	FSM_STATE* m_current_state = nullptr;
	FSM_STATE* m_state_ramaser_halite = nullptr;
	FSM_STATE* m_state_move_to_halite = nullptr;
	FSM_STATE* m_state_move_to_storage = nullptr;
	FSM_STATE * m_state_move_to_enemie = nullptr;
};