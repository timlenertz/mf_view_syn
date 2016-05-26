/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
