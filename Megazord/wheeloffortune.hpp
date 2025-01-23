#pragma once
#include <string>
#include <vector>
/*
	Classe générique pour les roues de la fortune
*/

class Wheel {
public:
	Wheel();

	~Wheel();

	void addEvent(std::string, float);

	std::string roll();

	void clear();

private:

	std::vector<std::pair<std::string, float>> m_events;

	float m_sum;

};