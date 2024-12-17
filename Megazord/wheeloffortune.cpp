#include "wheeloffortune.hpp"

Wheel::Wheel()
{
	m_sum = 0;
}

Wheel::~Wheel()
{

}

void Wheel::addEvent(std::string _name, float _chance)
{
	m_events.push_back({ _name, _chance });
	m_sum += _chance;
}

void Wheel::clear()
{
	m_events.clear();
	m_sum = 0;
}

std::string Wheel::roll()
{
	float dice = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / m_sum));
	float cumsum = 0;
	for (auto& event_pair : m_events)
	{
		cumsum += event_pair.second;
		if (dice < cumsum)
		{
			return event_pair.first;
		}
	}
	return m_events[m_events.size()].first;
}