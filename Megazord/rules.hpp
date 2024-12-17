#pragma once
#include "utils.hpp"
#include <vector>
#include <string>
#include <tuple>

/*
	Classe générique pour la gestion de règles
*/

typedef void(*void_function_voidp)(void*);

typedef std::pair<std::string, void_function_voidp> rule_t;
typedef std::pair<std::string, float> desire_t;

class Rules {
public:
	Rules(std::string, void*);

	~Rules();

	std::string infer();

	void addRule(std::string, void_function_voidp);

	void setDesire(std::string, float);

	float getDesire(std::string);

	void debugLog(std::string);

	void* m_parent;

private:

	int getRawDesire(std::string);

	std::vector<rule_t> m_rules;

	std::vector<desire_t> m_desires;

	std::string m_name;

};
