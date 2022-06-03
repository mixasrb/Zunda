#pragma once
#include "MBC.h"

class MBC1 : public MBC
{
public:
	MBC1();

	uint8_t CpuRead(uint16_t addr, std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);
	void CpuWrite(uint16_t addr, uint8_t data, std::vector<uint8_t>& ram);
private:
	bool bRam = false;
	uint8_t selected_bank;
	uint8_t selected_ram_bank;
	uint8_t bank1 = 0;
	uint8_t bank2 = 0;
	uint8_t mode;
};