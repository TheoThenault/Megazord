#include "rules.hpp"
#include "../hlt/log.hpp"
#include "wheeloffortune.hpp"

Rules::Rules(std::string _name, void* _parent) : m_name(_name), m_parent(_parent)
{

}

Rules::~Rules()
{

}

std::string Rules::infer()
{
	m_desires.clear();

	for (rule_t rule : m_rules)
	{
		std::string rule_name = rule.first;
		void_function_voidp rule_function = rule.second;

		debugLog("Start of rule : " + rule_name);

		rule_function(m_parent);

		debugLog("End of rule : " + rule_name);
	}

	// Dont roll the wheel without any events
	if (m_desires.size() == 0)
		return "null";

	// Choose outcome
	Wheel wheel;
	for (desire_t d : m_desires)
	{
		LOG("Desire " + d.first + " : " + std::to_string(d.second));
		wheel.addEvent(d.first, d.second);
	}

	return wheel.roll();
}

void Rules::addRule(std::string _name, void_function_voidp _func)
{
	m_rules.push_back({ _name, _func });

	debugLog("New rule : " + _name);
}

void Rules::setDesire(std::string _name, float _val)
{
	int index = getRawDesire(_name);
	if (index == -1)
		m_desires.push_back({ _name, _val });
	else
		m_desires[index].second = _val;


	debugLog("{" + _name + ", " + std::to_string(_val) + "}");
}

float Rules::getDesire(std::string _name)
{
	int existing_desire = getRawDesire(_name);

	if (existing_desire == -1)
	{
		return 0;
	}
	else {
		return m_desires[existing_desire].second;
	}
}

int Rules::getRawDesire(std::string _name)
{
	for (unsigned int i = 0; i < m_desires.size(); i++)
	{
		desire_t desire = m_desires[i];
		if (desire.first == _name)
		{
			return i;
		}
	}
	return -1;
}

void Rules::debugLog(std::string str)
{
	LOG("[" + m_name + "] " + str);
}
