#pragma once
#include "rules.hpp"
#include <map>

/*
	Classe spécifique pour les comportement du "joueur"
*/

class Joueur {
	friend class Rules;

public:

	Joueur();

	~Joueur();

	void think();

	int m_value;

	void spawnBoat();

	Rules* m_rule_engine;

private:

};