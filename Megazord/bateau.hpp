#pragma once

#include "../hlt/game.hpp"
#include "fsm.hpp"
#include "joueur.hpp"

/*
	Classe spécifique pour le comportement des bateaux
*/

class Bateau {
public:
	Bateau(hlt::Game*, std::shared_ptr<hlt::Player>, Joueur*, hlt::EntityId);
	~Bateau();
	
	hlt::Game* m_game;
	std::shared_ptr<hlt::Player> m_player;
	Joueur* m_joueur;
	hlt::EntityId m_ship_id;
	FSM* m_state_machine;

	hlt::Position m_target_storage;
	hlt::Position m_target_enemie;

	std::vector <hlt::Command>* command_queue;
	std::shared_ptr <hlt::Ship> ship;

	void decide(std::vector <hlt::Command>*, std::shared_ptr <hlt::Ship>);
	void collectHalites();
	void moveToHalites();
	void moveToTarget();
	void transformDropoff();

private:
	void setupStateMachine();

	std::string lastState = "CREATION";

	FSM_STATE*	m_current_state			= nullptr;
	FSM_STATE*	m_state_ramaser_halite	= nullptr;
	FSM_STATE*	m_state_move_to_halite	= nullptr;
	FSM_STATE*	m_state_move_to_storage	= nullptr;
	FSM_STATE*	m_state_move_to_enemie	= nullptr;
	FSM_STATE*	m_state_create_dropoff	= nullptr;
};

int shipIndex(std::vector<Bateau*>*, hlt::EntityId);
