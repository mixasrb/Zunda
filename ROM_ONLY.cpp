#include "ROM_ONLY.h"

// CPU

uint8_t ROM_ONLY::CpuRead(uint16_t addr, std::vector<uint8_t>& rom, std::vector<uint8_t>& ram)
{
	uint8_t data = 0xFF;
	data = rom[addr];
	return data;
}

void ROM_ONLY::CpuWrite(uint16_t addr, uint8_t data, std::vector<uint8_t>& ram)
{
	// Nothing
}
