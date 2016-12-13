#include <fstream>
#include <sstream>
#include <iostream>
#include "lsystem.h"

std::string loadGrammar(int& num_iter, float& angle, std::string& axiom,
	std::map<char, std::string>& grammar, char* filename) {
	
	std::stringstream err;
	std::ifstream ifs(filename);
	if (!ifs) {
		err << "Cannot open file " << filename << std::endl;
		return err.str();
	}

	std::string line;
	std::getline(ifs, line);
	num_iter = std::stoi(line);
	std::getline(ifs, line);
	angle = std::stof(line);
	std::getline(ifs, line);
	axiom = line;
	std::string rule;
	while (std::getline(ifs, line)) {
		if (line.length() > 2) {
			rule = line.substr(2);
			grammar.insert(std::pair<char, std::string>(line[0], rule));
		}
	}

	return err.str();
}