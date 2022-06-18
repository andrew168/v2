#pragma once
#include "..\vk-all.h"
#include "string"

namespace aux
{
class Log {
public:
	static bool enabled;
	static void info(const std::string& msg);
	static void fatal(const std::string& msg);
};
}