#include "log.h"
#include "iostream"

namespace aux
{
bool Log::enabled = true;

void Log::info(const std::string& msg) {
	std::cout << msg;
}

void Log::fatal(const std::string& msg)
{
	std::cerr << msg << "\n";
	if (!enabled) {
		MessageBox(NULL, msg.c_str(), NULL, MB_OK | MB_ICONERROR);
	}
}

}
