#include "joueur.hpp"
#include "../hlt/log.hpp"

static void rule1(void* _joueur)
{
	Joueur* joueur = (Joueur*)_joueur;
	if (joueur->m_value > 10)
	{
		joueur->m_rule_engine->setDesire("SpawnBoat", 1);
	}
}

static void rule2(void* _joueur)
{
	Joueur* joueur = (Joueur*)_joueur;
	joueur->m_value++;
}

Joueur::Joueur()
{
	m_value = 0;

	m_rule_engine = new Rules("Joueur", (void*)this);

	m_rule_engine->addRule("rule1", rule1);
	m_rule_engine->addRule("rule2", rule2);
}

Joueur::~Joueur()
{
	delete m_rule_engine;
}

void Joueur::think()
{
	std::string action = m_rule_engine->infer();

	if (action == "SpawnBoat")
	{
		spawnBoat();
	}
}


void Joueur::spawnBoat()
{
	hlt::log::log("SPAWN BOAT");
}