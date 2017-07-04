#include <fstream>
#include <sstream>
#include <iostream>
#include "lsystem.h"


std::map<char, std::string> grammar;

std::string applyRules(char c) {
	if (grammar.count(c)) {
		return grammar.at(c);
	}
	else {
		return std::string(1, c);
	}
}

std::string processString(std::string oldString) {
	std::string newString = "";
	for (char c : oldString) {
		std::string a = applyRules(c);
		newString += a;
	}
	return newString;
}

std::string createLSystem(int n, std::string axiom) {
	std::string newString = axiom;
	for (int i = 0; i < n; i++) {
		newString = processString(newString);
	}
	return newString;
}

std::string loadLSystem(std::string& lSystem, float& angle, char* filename) {
	
	std::stringstream err;
	std::ifstream ifs(filename);
	if (!ifs) {
		err << "Cannot open file: " << filename << std::endl;
		return err.str();
	}

	grammar.clear();

	try {
		std::string line;
		std::getline(ifs, line);
		int num_iter = std::stoi(line);
		std::getline(ifs, line);
		angle = std::stof(line);
		std::getline(ifs, line);
		std::string axiom = line;
		std::string rule;
		while (std::getline(ifs, line)) {
			if (line.length() > 2) {
				rule = line.substr(2);
				grammar.insert(std::pair<char, std::string>(line[0], rule));
			}
		}

		lSystem = createLSystem(num_iter, axiom);
	}
	catch (std::invalid_argument&) {
		err << "File '" << filename << "' is not in proper format" << std::endl;
	}
	

	return err.str();
}