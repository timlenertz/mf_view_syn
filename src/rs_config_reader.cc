#include "rs_config_reader.h"
#include <sstream>
#include <fstream>

namespace vs {

void rs_config_reader::parse_(std::istream& str) {
	std::string line;
	while(std::getline(str, line)) {
		if(line.empty() || line.front() == '#') continue;
		
		std::istringstream ss(line);
		std::string key, value;
		ss >> key >> value;
		
		parameters_[key] = value;
	}
}


rs_config_reader::rs_config_reader(const std::string& filename) {
	std::ifstream str(filename);
	parse_(str);
}


bool rs_config_reader::has(const std::string& key) const {
	auto it = parameters_.find(key);
	return (it != parameters_.end());
}

	
const std::string& rs_config_reader::get_string(const std::string& key) const {
	auto it = parameters_.find(key);
	if(it != parameters_.end()) return it->second;
	else throw std::invalid_argument("parameter not in configuration file");
}


int rs_config_reader::get_int(const std::string& key) const {
	return std::stoi(get_string(key));
}


mf::real rs_config_reader::get_real(const std::string& key) const {
	return std::stod(get_string(key));
}


}
