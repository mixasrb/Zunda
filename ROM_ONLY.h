#pragma once
#include "MBC.h"

class ROM_ONLY : public MBC
{
public:
	uint8_t CpuRead(uint16_t addr, std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);
	void CpuWrite(uint16_t addr, uint8_t data, std::vector<uint8_t>& ram);
};