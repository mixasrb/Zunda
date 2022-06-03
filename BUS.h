#pragma once
#include <iostream>
#include <cstdint>

#include "CPU.h"
#include "PPU.h"
#include "psg.h"
#include "BootRom.h"
#include "Cartridge.h"
#include "Log.h"

class BUS
{
public:
	BUS();

	const long int BASE_CLOCK = 1048576;

	// Connect CPU to the bus

	CPU cpu;

	// Connect PPU to the bus

	PPU ppu;

	psg apu;

	BootRom boot_rom;

	Cartridge cart;

	void InsertCartridge(const char* cart);

	// RAM
	//std::array<uint8_t, 8192> ram = { 0 };
	uint8_t* ram = new uint8_t[8192];

	// VRAM
	//std::array<uint8_t, 8192> vram = { 0 };
	uint8_t* vram = new uint8_t[8192];

	// OAM
	uint8_t* OAM = new uint8_t[0xA0];

	// Serial tansfer data
	uint8_t SB = 0x00;

	// S10 Control
	uint8_t SC = 0x00;

	// DIV Register
	uint8_t DIV = 0x00;

	// TIMA Register
	uint8_t TIMA = 0x00;
	bool bTIMAoverflow = false;
	int help_cycles = 0;

	// TMA Register
	uint8_t TMA = 0x00;

	// TAC Register
	uint8_t TAC = 0x00;
	uint16_t tPeriod = 0;

	// BOOT Turn Off Register
	uint8_t BOOT = 0x00;

	// HRAM
	//std::array<uint8_t, 127> hram = { 0 };
	uint8_t* hram = new uint8_t[127];

	// P1 (Controller)
	uint8_t p1 = 0xFF;

	// Interrupt Enable Register

	union IE_Register // (R/W)
	{
		struct
		{
			uint8_t V_Blank : 1;
			uint8_t LCDC : 1;
			uint8_t Timer_Overflow : 1;
			uint8_t Serial_I_O : 1;
			uint8_t P10_P13 : 1;
			uint8_t unused : 3;
		};
		uint8_t data = 0xE0;
	}IF, IE;


	uint8_t CpuRead(uint16_t addr);
	void CpuWrite(uint16_t addr, uint8_t data);

	uint8_t PpuRead(uint16_t addr);

	void start();
	bool Clock();

	unsigned long int SystemCycleCounter = 0;

	bool bButtonPressed = false;

	uint16_t divider = 0;
	bool bLast;
	bool bTMA = false;
	long int nTimerCounter = 0;

	uint8_t DMA_Clocks = 0;

	// Helper Variables

	// Debugging utility

	Log log;
	long int past_cycles = 0;
	uint8_t serial;
	bool bLast_serial = false;
	uint8_t c = 0;

	bool bSampleReady;
	float fTime = 0.0f;
	float fSample;
	bool bOverflow;
};