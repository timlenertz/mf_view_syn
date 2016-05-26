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

#include <cstddef>
#include <iostream>

#include "view_synthesis.h"

using namespace vs;

int main(int argc, const char* argv[]) {
	std::cout << "mf: view_syn\n" << std::endl;
	
	if(argc < 2) {
		std::cout << "must provide configuration file argument!" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::cout << "setting up..." << std::endl;
	view_synthesis syn(argv[1]);
	syn.setup();
	
	std::cout << "running..." << std::endl;
	syn.run();
	
	std::cout << "finished." << std::endl;
	return EXIT_SUCCESS;
}
