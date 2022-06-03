#include "MBC1.h"

// CPU
MBC1::MBC1()
{
	selected_bank = 1;
}

uint8_t MBC1::CpuRead(uint16_t addr, std::vector<uint8_t>& rom, std::vector<uint8_t>& ram)
{
	uint8_t data = 0x00;

	if (mode)
		selected_bank = (uint8_t)bank2 << 5 | bank1;
	else
		selected_bank = bank1;

	if (mode)
		selected_ram_bank = 0;
	else
		selected_ram_bank = bank2;

	if (!selected_bank)
	{
		selected_bank = 1;
	}
	//selected_bank &= 0x05;

	if (addr >= 0x0000 && addr <= 0x3FFF)
	{
		data = rom[addr];
	}
	else if (addr >= 0x4000 && addr <= 0x7FFF)
	{
		uint32_t address = addr - 0x4000 + ((uint32_t)selected_bank) * 0x4000;
		data = rom[address];
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		if (bRam)
		{
			uint32_t address = addr - 0xA000 + ((uint32_t)selected_ram_bank) * 0x2000;
			data = ram[address];
		}
		else
			data = 0xFF;
	}
	return data;
}

void MBC1::CpuWrite(uint16_t addr, uint8_t data, std::vector<uint8_t>& ram)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		if ((data & 0x0F) == 0x0A)
			bRam = true;
		else
			bRam = false;
	}
	else if (addr >= 0x2000 && addr <= 0x3FFF)
	{
		bank1 = data & 0x1F;
		bank1 = bank1 ? bank1 : 1;
	}
	else if (addr >= 0x4000 && addr <= 0x5FFF)
	{
		bank2 = data & 0x03;
	}
	else if (addr >= 0x6000 && addr <= 0x7FFF)
	{
		mode = data & 0x01;
		//printf("Model : %x %x\n", addr, data);
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		if (bRam)
		{
			if (mode)
				selected_ram_bank = 0;
			else
				selected_ram_bank = bank2;
			uint32_t address = addr - 0xA000 + ((uint32_t)selected_ram_bank) * 0x2000;
			ram[address] = data;
		}
	}
}