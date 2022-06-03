#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>

class Bus;

class BootRom
{
public:

	BootRom();

	uint8_t bios_data[256] = {0};
};