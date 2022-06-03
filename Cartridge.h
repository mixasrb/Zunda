#pragma once
#include <math.h>

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>

#include "MBC.h"
#include "ROM_ONLY.h"
#include "MBC1.h"

class Cartridge
{
public:

	const char* cart_name;

	void ReadCart();

	uint8_t* reserved = new uint8_t[0x0150];
	std::vector<uint8_t> rom;

	std::vector<uint8_t> ram;

public:
	struct header
	{
		char title[15];
		uint8_t color_GB;
		uint16_t licence_code;
		uint8_t indicator;
		uint8_t cart_type;
		uint8_t rom_size;
		uint8_t ram_size;
		bool destination_code;
		uint8_t old_licence_code;
		uint8_t mask_rom_number;
		uint8_t complement_check;
		uint8_t checksum[2];
	}m_header;

	void CpuRead(uint16_t addr, uint8_t& data);
	void CpuWrite(uint16_t addr, uint8_t data);

	MBC** mbc = new MBC*;
};