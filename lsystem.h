#pragma once

#include <map>

std::string loadGrammar(int& num_iter, float& angle, std::string& axiom,
	std::map<char, std::string>& grammar, char* filename);