#pragma once
#include <cstdint>
#include <vector>

class MBC
{
protected:
	uint8_t nRomBanks;
	uint8_t nRamBanks;

public:
	virtual uint8_t CpuRead(uint16_t addr, std::vector<uint8_t>& rom, std::vector<uint8_t>& ram) = 0;
	virtual void CpuWrite(uint16_t addr, uint8_t data, std::vector<uint8_t>& ram) = 0;

private:
	uint8_t selected_bank;
	uint8_t bank1;
	uint8_t bank2;
	uint8_t mode;
};