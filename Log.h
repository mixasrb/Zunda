#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

class Log
{
public:

	std::ofstream LogFile;

	Log()
	{
		LogFile.open("Log.txt", std::fstream::out);
		if (!LogFile.is_open())
			std::cout << "Cannot create log file!" << std::endl;
	}

	~Log()
	{
		LogFile.close();
	}

	void TraceLog(const std::string opcode, const int ppuCycle, const int ppuScanline,
		const unsigned long int System_counter)
	{
		std::string entry = opcode + " C:" + Convert(ppuCycle) + " S:" + Convert(ppuScanline) +
			" System Counter:" + Convert(System_counter) + "\n";
		LogFile << entry;
	}

	std::string Convert(long long int d)
	{
		std::string str;
		std::stringstream ss;
		ss << d;
		ss >> str;

		return str;
	}
};