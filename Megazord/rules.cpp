#include "rules.hpp"
#include "../hlt/log.hpp"

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

		debug_log("Start of rule : " + rule_name);

		rule_function(m_parent);

		debug_log("End of rule : " + rule_name);
	}

	// Choose outcome
	// TODO : Change to wheel of fortune
	std::string max_name	= "null";
	float		v_max = 0;
	for (desire_t d : m_desires)
	{
		if (d.second > v_max)
		{
			v_max = d.second;
			max_name = d.first;
		}
	}

	return max_name;
}

void Rules::addRule(std::string _name, void_function_voidp _func)
{
	m_rules.push_back({ _name, _func });

	debug_log("New rule : " + _name);
}

void Rules::setDesire(std::string _name, float _val)
{
	m_desires.push_back({ _name, _val });

	debug_log("{" + _name + ", " + std::to_string(_val) + "}");
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

void Rules::debug_log(std::string str)
{
#ifdef _DEBUG
	hlt::log::log("[" + m_name + "] " + str);
#endif
}
