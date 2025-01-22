#pragma once
#include "rules.hpp"
#include "../hlt/game.hpp"
#include "../hlt/player.hpp"
#include "../hlt/command.hpp"
#include <map>

/*
	Classe sp�cifique pour les comportement du "joueur"
*/

class Joueur {
public:

	Joueur(hlt::Game*);

	~Joueur();

	void setCommandQueue(std::vector<hlt::Command>*);

	void think(std::shared_ptr<hlt::Player>);

	void spawnBoat();

	void createDropoff();

	hlt::Game* m_game;

	std::shared_ptr<hlt::Player> m_player;

	Rules* m_rule_engine;
	
	int m_value;

	int m_expected_halite;

	int boatAboutToTransform = -1;

private:

	std::vector<hlt::Command>* m_command_queue;

};