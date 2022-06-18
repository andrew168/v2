#include "util.h"
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

namespace aux
{
bool fileExists(const std::string& filename)
{
	std::ifstream f(filename.c_str());
	return !f.fail();
}

}
